<?php
/**
 * @file
 * Renders the various source pages HTML into standard Drupal page output.
 */

/**
 * Loads the HTML content based on file URI.
 *
 * @param string $uri
 *   The HTML files full absolute path.
 *
 * @return string
 *   Properly encoded HTML contents from the file.
 */
function project_page_compiler_load_content($uri) {
  $contents = file_get_contents($uri);

  // Must convert all HTML to UTF-8 to properly process it.
  if (mb_detect_encoding($uri) != 'UTF-8') {
    $contents = utf8_encode($contents);
  }

  // Must cleanup badly formatted HTML to properly process it.
  $contents = preg_replace_callback("/([^>]*)/u", 'project_page_compiler_clean_html', $contents);

  return $contents;
}

/**
 * Encodes a source page HTML contents to UTF-8 if not otherwise.
 *
 * @param string $html
 *   The HTML that appears on the page and that is processed.
 *
 * @return string
 *   The converted text that is encoded to UTF-8.
 */
function project_page_compiler_encode($html) {
  if (!mb_check_encoding($cache[$key][$page_id]['uri'], 'UTF-8')) {
    $html = utf8_encode($html);
  }
  return $html;
}

/**
 * Cleans an HTML tag so it is ready for processing.
 *
 * @param array $match
 *   The HTML tag in the content that was found.
 */
function project_page_compiler_clean_html($match) {
  return trim($match[0], "\n");
}

/**
 * Gets index page if exists & no argument passed in URL
 *
 * @param string $type
 *   Type of page, howto or manpage
 *
 * @return string or null
 *   URL to assume as index or null if index provide elsewhere (ie. panels)
 */
function project_page_compiler_index_page($type) {
  $index_pages = array(
    'howto' => 'HOWTO-INDEX/howtos.html',
    'manpage' => NULL
  );
  return $index_pages[$type];
}

/**
 * Renders a source page HTML in a Drupal page.
 */
function project_page_compiler_render($file = '') {
  $cache = project_page_compiler_load_cache();
  $key = arg(0); // The type of source HTML.
  $body = '';

  if (!empty($file)) {
    // Load the HTML source type and file path.
    $file = explode('?', trim($_GET['q'], '/'));
    $file = explode('/', $file[0]);
    unset($file[0]);
    $file = implode('/', $file);
  }
  else if ($file = project_page_compiler_index_page($key)) {
    // Index page redirect, redirect so links aren't broken in content
    drupal_goto($key . '/' . $file);
  }
  // Load the page ID and validate the HTML file exists.
  $page_id = project_page_compiler_clean_filename($file);
  if (empty($file) || !project_page_compiler_verify_page($page_id, $key, $file, $cache)) {
    // The page file was not found.
    $func_404 = 'project_page_compiler_' . $key . '_404_page';
    if (function_exists($func_404)) {
      call_user_func($func_404);
    }
    else {
      call_user_func('project_page_compiler_404_page');
    }
    drupal_exit(); // Prevents double page output on 404 error.
  }

  // Load the contents of the HTML file.
  $contents = project_page_compiler_load_content($cache[$key][$page_id]['uri']);

  // Load the page title between the <title> tags.
  if (preg_match('/(?:<title[^>]*>)(.*)<\/title>/isU', $contents, $matches)) {
    $contents = preg_replace('/(?:<h1[^>]*>)' . $matches[1] . '<\/h1>/isU', '' , $contents);
    drupal_set_title($matches[1]);
  }
  // Load the page content between the <body> tags.
  if (preg_match('/(?:<body[^>]*>)(.*)<\/body>/isU', $contents, $matches)) {
    $body = $matches[1];
  }

  // Update MAN page links to point to appropriate directories.
  if ($key == 'manpage') {
    $body = preg_replace_callback('/(?:<a[^>]*>)(.*)<\/a>/isU', 'project_page_compiler_manpage_links', $body);
  }

  // Update HOWTO page images to point to source directories.
  else if ($key == 'howto') {
    $body = preg_replace_callback('/(<img[^>]+>)/isU', 'project_page_compiler_howto_images', $body);
  }

  // Update Previous Next links

  $body = preg_replace_callback('/(Next|Previous|Contents)/isU', 'project_page_compiler_manpage_previous_next', $body);

  return $body;
}

function project_page_compiler_manpage_previous_next($match) {
  return '<span class="navigation-link">' . $match[0] . '</span>';
}

/**
 * Transforms MAN page URLs into clickable Drupal links.
 *
 * @param array $match
 *   The link in the MAN page content that was found.
 */
function project_page_compiler_manpage_links($match) {
  $path_prefix = 'http://localhost/cgi-bin/man/man2html';
  $path_pattern = '/href=("|\')(.*)("|\')/isU';
  if (preg_match($path_pattern, $match[0], $matches)) {
    // Replacement for anchor links.
    if ($matches[2] == $path_prefix) {
      if ($match[1] == 'Return to Main Contents') {
        $match[0] = '';
        $match[1] = '';
      }
      else {
        $match[0] = $match[1];
      }
      return preg_replace($path_pattern, 'href="#"', $match[0]);
    }
    // Replacement for page links.
    else if (strpos($matches[2], $path_prefix) !== false) {
      $page_link = array_reverse(explode('+', str_replace(array('?'), array(''), str_replace($path_prefix, '', $matches[2]))));
      $url = url('manpage/man'. $page_link[1] .'/'. implode('.', $page_link) .'.html');
      return preg_replace($path_pattern, 'href="' . $url . '"', $match[0]);
    }
    // file:/// links, remove link
    else if (strpos($match[0], 'file:///') !== false) {
      return $match[1];
    }
  }

  return $match[0];
}

/**
 * Transforms HOWTO image URLs into valid Drupal paths.
 *
 * @param array $match
 *   The image in the HOWTO page content that was found.
 */
function project_page_compiler_howto_images($match) {
  $src_pattern = '/src=("|\')([^("|\')]*)("|\')/i';
  if (preg_match($src_pattern, $match[0], $matches)) {
    $image_src = $matches[2];
    $full_path = explode('/', $_GET['q']);
    unset($full_path[0], $full_path[count($full_path)]);

    // Convert relative paths to absolute.
    if (strpos($image_src, '../') !== false) {
      preg_match_all('/..\//i', $matches[2], $back_steps);
      $back_steps = count($back_steps);
      for ($i = 1; $i <= $back_steps; $i++) {
        unset($full_path[$i]);
      }
      $image_src = str_replace('../', '', $image_src);
    }

    // Build the full path to the image.
    $image_path = trim(implode('/', $full_path) .'/'. $image_src, '/');
    return preg_replace($src_pattern, 'src="/page/image?path='. $image_path .'"', $match[0]);
  }
  return $match[0];
}

/**
 * Manpage 404 message
 */
function project_page_compiler_404_page() {
  drupal_set_title(t('Page not found'));
  $content = t("We're sorry, that page could not be found. Try this <a href=\"@man_howto_search\">site's search functionality</a> instead.", array('@man_howto_search' => url('search/man-howto')));
  return drupal_deliver_page($content);
}

/**
 * Manpage 404 message
 */
function project_page_compiler_manpage_404_page() {
  drupal_set_title(t('Manpage not found'));
  $content = t("We're sorry, that manpage isn't part of the Kernel.org project Programmer's Manual. If you have project installed, you can try using the \"man\" command in the shell or use this <a href=\"@man_howto_search\">site's search functionality</a>.", array('@man_howto_search' => url('search/man-howto')));
  return drupal_deliver_page($content);
}
