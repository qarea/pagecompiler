<?php
/**
 * @file
 * Provides integration with project source pages and Drupal view displays.
 */

/**
 * Implements phptemplate_preprocess_views_view_field().
 * Adds the appropriate source page title and body to views field.
 */
function project_page_compiler_preprocess_views_view_field(&$vars) {

  // This should only ever apply to the "source_page" entity type, which is
  // defined in hook_search_api_item_type_info() of this module.
  if (!empty($vars['field']->entity_type) && $vars['field']->entity_type == 'source_page') {
    $output = '';
    $field = $vars['field']->field;
    $cache = project_page_compiler_load_cache();
    $page_id = $vars['row']->entity;

    // Determine the source key for this page ID.
    $key = project_page_compiler_find_key($page_id);

    // Check that this source page still exists and is accessible.
    if (!project_page_compiler_verify_page($page_id, $key, $cache[$key][$page_id]['page_file'], $cache)) {
      project_page_compiler_delete_index($page_id);
      return;
    }

    // Load the contents of the HTML file.
    $contents = project_page_compiler_load_content($cache[$key][$page_id]['uri']);

    // Produce the Drupal path to this page.
    $url = $key . str_replace(variable_get('project_'. $key .'_source', ''), '', $cache[$key][$page_id]['uri']);

    // Load the page title between the <title> tags.
    if ($field == 'page_title' && preg_match('/(?:<title[^>]*>)(.*)<\/title>/isU', $contents, $matches)) {
      $output = '<h3>'. l($matches[1], $url) .'</h3>';
    }

    // Load the page content between the <body> tags.
    if ($field == 'page_content' && preg_match('/(?:<body[^>]*>)(.*)<\/body>/isU', $contents, $matches)) {
      $excerpt_keywords = array();
      foreach ($vars['view']->exposed_raw_input as $keywords) {
        $excerpt_keywords[] = $keywords;
      }
      $output  = '<p>'. project_page_compiler_excerpt(implode(' ', $excerpt_keywords), $matches[1]) .'</p>';

      $file_stat = stat($cache[$key][$page_id]['uri']);
      $output .= '<footer><em>'. format_date($file_stat['mtime']) .'</em></footer>';
    }

    // TEMPORARY: TO REVEAL DISPLAY ISSUES.
    if (empty($output)) {
      drupal_set_message('Error displaying: '. $cache[$key][$page_id]['page_file']);
    }
    $vars['output'] = $output;
  }
}
