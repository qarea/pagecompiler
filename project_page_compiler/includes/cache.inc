<?php
/**
 * @file
 * Provides caching routines for source pages.
 */

/**
 * Loads all the available project source paths.
 */
function project_page_compiler_load_sources() {
  return array(
    'howto' => variable_get('project_howto_source', ''),
    'manpage' => variable_get('project_manpage_source', ''),
  );
}

/**
 * Loads all source HTML files and caches the information.
 */
function project_page_compiler_cache_set() {
  $cache = array();
  $total = 0;
  $files = array();

  // Refresh files cache
  variable_set('project_page_files_cache', array());

  // Get source path
  $source_paths = project_page_compiler_load_sources();

  // Load all .HTML files inside the source directory (recursively).
  foreach ($source_paths as $key => $path) {
    $files[$key] = file_scan_directory($path, '/\.html/', array('recurse' => TRUE));
  }

  // Loop through files and create cache entry.
  foreach ($files as $key => $pages) {
    foreach ($pages as $html) {

      // Store each page with a unique page ID based on the full URI. This is
      // translated into an MD5 hash for identification.
      $page_file = trim(str_replace($source_paths[$key], '', $html->uri), '/');
      $page_id = project_page_compiler_clean_filename($page_file);
      $page_stat = stat($html->uri); // Gather file statistics (such as last modified).

      // Create the cache entry for this HTML file.
      $cache[$key][$page_id] = array(
        'uri' => $html->uri,
        'file' => $html->filename,
        'page_file' => $page_file,
        'modified' => $page_stat['mtime'],
      );

      // Create index entry if one does not already exist for the page.
      $lookup = db_select('search_api_index_project_sources', 'l')->fields('l')->condition('item_id', $page_id, '=');
      if (!$lookup->countQuery()->execute()->fetchField()) {
        search_api_track_item_insert('source_page', array($page_id));
      }
    }
  }

  // Store the cached HTML pages into Drupal for usage.
  variable_set('project_page_cache', $cache);
  foreach ($cache as $key => $pages) {
    $total += count($pages);
  }
  drupal_set_message(t('A total of %num HTML source pages have been cached.', array('%num' => number_format($total))));
}

/**
 * Verify if a page exists but has not yet been cached.
 *
 * @param hash $page_id
 *   The source page MD5 hash string that identifies the page.
 *
 * @param string $key
 *   Type of source page that is being verified.
 *
 * @param string $file
 *   The relative filepath of the source page in question.
 *
 * @param array $cache
 *   Array of cached pages passed through as a reference.
 *
 * @return boolean
 *   Boolean value when TRUE is verified and FALSE is not.
 */
function project_page_compiler_verify_page($page_id, $key, $file, &$cache) {
  $file_path = variable_get('project_'. $key .'_source', '') .'/'. $file;
  if (project_page_compiler_file_exists($file_path)) {
    $source_paths = project_page_compiler_load_sources();

    // Load additional metadata about the file.
    $file_info = pathinfo($file_path);
    $page_stat = stat($file_path); // Gather file statistics (such as last modified).

    // Create the cache entry for this HTML file.
    $cache[$key][$page_id] = array(
      'uri' => $file_path,
      'file' => $file_info['filename'],
      'page_file' => $file,
      'modified' => $page_stat['mtime'],
    );
    variable_set('project_page_cache', $cache);

    // Insert new page into the search index IDs.
    project_page_compiler_insert_index($page_id);

    return TRUE;
  }

  // Remove the entry from the $cache variable if it exists.
  if (isset($cache[$key][$page_id])) {
    unset($cache[$key][$page_id]);
    variable_set('project_page_cache', $cache);
  }

  // Delete old index entry, if one exists.
  project_page_compiler_delete_index($page_id);

  return FALSE;
}
