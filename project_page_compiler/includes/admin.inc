<?php
/**
 * @file
 * Administration settings for project page compiler.
 */

/**
 * Form callback for project page compiler settings.
 */
function project_page_compiler_admin_settings() {
  $cache = project_page_compiler_load_cache();

  // Display statistic regarding total number of source pages found.
  if (!empty($cache)) {
    $total = 0;
    foreach ($cache as $key => $pages) {
      $total += count($pages);
    }
    drupal_set_message(t('There are a total of !num source pages.', array('!num' => number_format($total))));
  }

  // Source paths
  $form['sources'] = array(
    '#type' => 'fieldset',
    '#title' => t('Source paths'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['sources']['project_howto_source'] = array(
    '#type' => 'textfield',
    '#title' => t('Absolute path to HOWTOs source HTML files:'),
    '#default_value' => variable_get('project_howto_source', ''),
    '#description' => !empty($cache['howto']) ? t('There are a total of !num HOWTO pages.', array('!num' => number_format(count($cache['howto'])))) : t('Example: /Volumes/DRIVE/Sites/project/dev/sites/default/files/sources/HOWTO-a'),
  );
  $form['sources']['project_manpage_source'] = array(
    '#type' => 'textfield',
    '#title' => t('Absolute path to MAN source HTML files:'),
    '#default_value' => variable_get('project_manpage_source', ''),
    '#description' => !empty($cache['manpage']) ? t('There are a total of !num MAN pages.', array('!num' => number_format(count($cache['manpage'])))) : t('Example: /Volumes/DRIVE/Sites/project/dev/sites/default/files/sources/man-html-a'),
  );
  $form['sources']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );
  $form['sources']['refresh_cache'] = array(
    '#type' => 'submit',
    '#value' => t('Manually refresh source cache'),
  );

  // Search indexing
  $form['index'] = array(
    '#type' => 'fieldset',
    '#title' => t('Search indexing'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => '<p>'. t('Use the following tools with caution as they cannot be undone:') .'</p>',
  );
  $form['index']['refresh_index'] = array(
    '#type' => 'submit',
    '#value' => t('Refresh the index page items'),
  );
  $form['index']['repair_index_table'] = array(
    '#type' => 'submit',
    '#value' => t('Clean and repair index table'),
  );

  return $form;
}

/**
 * Re-detects the source pages that need to be indexed.
 *
 * @param boolean $truncate
 *   Deletes previous index entries before refreshing if TRUE.
 */
function project_page_compiler_refresh_indexes($truncate = FALSE) {

  // Remove all the pre-existing items that have been detected for indexing.
  if ($truncate) {
    db_query("TRUNCATE TABLE {search_api_index_project_sources}");
  }

  // Update the page cache before adding new items.
  project_page_compiler_cache_set();
  $cache = project_page_compiler_load_cache();

  // Create new index entries for the updated files cache.
  foreach ($cache as $key => $pages) {
    foreach ($pages as $page_id => $html) {
      $lookup = db_select('search_api_index_project_sources', 'l')->fields('l')->condition('item_id', $page_id, '=');
      if (!$lookup->countQuery()->execute()->fetchField()) {
        search_api_track_item_insert('source_page', array($page_id));
      }
    }
  }
}

/**
 * Saves project source path and caches HTML files.
 */
function project_page_compiler_admin_settings_submit($form, $form_state) {
  $action = $form_state['clicked_button']['#id'];

  // Manually re-build the source search index.
  if ($action == 'edit-refresh-index') {
    project_page_compiler_refresh_indexes(TRUE); // Re-detects all items that can be indexed.
    drupal_set_message(t('The source page index has been refreshed, it is recommended that you re-index now.'));
  }

  // Re-create the project source pages index table.
  else if ($action == 'edit-repair-index-table') {
    drupal_uninstall_schema('project_page_compiler');
    drupal_install_schema('project_page_compiler');
    drupal_set_message(t('The index table "search_api_index_project_sources" has been re-built.'));
  }

  // Manually refresh the cache entries for source pages.
  else if ($action == 'edit-refresh-cache') {
    project_page_compiler_cache_set();
    drupal_set_message(t('The source page cache has been refreshed, changes take affect immediately.'));
  }

  // Cache all of the available project source HTML files.
  else {
    variable_set('project_howto_source', $form_state['values']['project_howto_source']);
    variable_set('project_manpage_source', $form_state['values']['project_manpage_source']);
    project_page_compiler_cache_set(); // Establishes the HTML file data.
  }
}
