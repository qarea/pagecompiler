<?php
/**
 * @file
 * Properly loads images included in the source HTML files.
 */

/**
 * Page callback; Loads images from HTML source directory.
 *
 * @return image
 *   An image loaded from the source directory.
 */
function project_page_compiler_load_image() {
  $cache = project_page_compiler_load_cache();
  if (!isset($_GET['path']) || empty($_GET['path'])) {
    drupal_not_found();
  }

  // Return 404 if the image does not exist.
  $image_path = variable_get('project_howto_source', '') .'/'. $_GET['path'];
  if (!file_exists($image_path)) {
    drupal_not_found();
  }

  // Load and deliver the image.
  $image_info = image_get_info($image_path);
  drupal_add_http_header('Content-Type', $image_info['mime_type']);
  readfile($image_path);
  drupal_exit();
}
