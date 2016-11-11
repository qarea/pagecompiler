<?php
/**
 * @file
 * Search API integration with the source pages.
 */

/**
 * Inserts a new index entry for a source page.
 *
 * @param hash $page_id
 *   The hash that identifies the source page.
 */
function project_page_compiler_insert_index($page_id) {
  project_page_compiler_delete_index($page_id);
  search_api_track_item_insert('source_page', array($page_id));
}

/**
 * Deletes an old index entry for a source page.
 *
 * @param hash $page_id
 *   The hash that identifies the source page.
 */
function project_page_compiler_delete_index($page_id) {
  search_api_track_item_delete('source_page', array($page_id));
}

/**
 * Search API DataSourceController for source page indexing.
 */
class projectPageDataSourceController extends SearchApiAbstractDataSourceController {
  protected $table = 'search_api_index_project_sources';

  /**
   * Defines item ID field info.
   */
  public function getIdFieldInfo() {
    return array(
      'key' => 'item_id',
      'type' => 'string',
    );
  }

  /**
   * Defines field properties for indexed items.
   */
  protected function getPropertyInfo() {
    $info['property info']['page_title'] = array(
      'label' => t('Title'),
      'type' => 'text',
    );
    $info['property info']['page_content'] = array(
      'label' => t('Content'),
      'type' => 'text',
    );
    $info['property info']['page_type'] = array(
      'label' => t('Page Type'),
      'type' => 'text',
    );    
    return $info;
  }

  /**
   * Load the items and related page content.
   */
  public function loadItems(array $pages) {
    $items = array();
    $cache = project_page_compiler_load_cache();

    // Load the page content for each page item.
    foreach ($pages as $page_id) {
      $index_object = new stdClass();
      $title = $body = '';

      // Determine the source key for this page ID.
      $key = project_page_compiler_find_key($page_id);

      // Validate this page entry before attempting to index.
      if (!project_page_compiler_verify_page($page_id, $key, $cache[$key][$page_id]['page_file'], $cache)) {
        project_page_compiler_delete_index($page_id); // Not a valid page, deleting the search index entry.
        continue; // Move on to the next source page.
      }

      // Load the contents of the HTML file.
      $contents = project_page_compiler_load_content($cache[$key][$page_id]['uri']);

      // Load the page title between the <title> tags.
      if (preg_match('/(?:<title[^>]*>)(.*)<\/title>/isU', $contents, $matches)) {
        $title = $matches[1];
      }

      // Load the page content between the <body> tags.
      if (preg_match('/(?:<body[^>]*>)(.*)<\/body>/isU', $contents, $matches)) {
        $body = $matches[1];
      }

      // Save the title/body into the index object.
      $index_object->page_title = $title;
      $index_object->page_content = $body;
      $index_object->page_type = $key;

      $items[$page_id] = $index_object;
    }
    return $items;
  }

  /**
   * Get a URL at which the item can be viewed on the web.
   */
  public function getItemUrl($item) {
    return NULL;
  }

  /**
   * Returns the page IDs of all project source HTML pages.
   */
  public function getAllItemIds() {
    $items = array();
    $cache = project_page_compiler_load_cache();
    foreach ($cache as $key => $pages) {
      $items = array_merge(array_keys($pages), $items);
    }
    return $items;
  }
}
