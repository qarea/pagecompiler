## project.com Page Compiler

The project.com Page Compiler creates Drupal pages from project source HTML files, without using entities or nodes. These source pages are searchable through the Search API module using a custom indexing class.

### Installation

* The project.com Page Compiler module is a stand-alone Drupal 7 module that has a dependency on the Search API module (for indexing).

### Configuration

* Enable the module and configure the HTML source paths at /admin/config/project/pages.
** Example: /Volumes/DRIVE/Sites/project/dev/sites/default/files/sources/HOWTO-a
* Clicking on "Save configuration" will scan the source paths entered and produce an initial cache of available pages.
** This step does not need to be repeated unless the source path location changes.
* Create a new Search API index for the source pages at /admin/config/search/search_api.
** Set the "Item type" for the new index to "project source page".
** Check both "page_title" and "page_content" fields to be included in the indexing.
** Check the "Ignore case" processor so searches are case-insensitive.
** Other processrors and settings can be applied at this point.
* Import the "searchapi_view.example" view at /admin/structure/views/import.
** Copy and paste the contents of the .example file into the "Paste view code here" textarea.
** Click on import and the view should create itself.
** The default URL for the view is /sources/search.

### Testing

* There are two methods to locate/display a compiled project source page.
** Use the search view that was imported to find records, click their titles to load the pages.
** All source pages share the same URL pattern, which is the directories in which the HTML files live plus the filename with ".html" on the end.
