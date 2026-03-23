# Alpha Pagination for Views

The Alpha Pagination for Views module enables you to add an alphabetical menu in the header or footer of a views display.

Alpha Pagination for Views was designed and written by [Michael Bagnall](https://www.drupal.org/u/elusivemind).

This module exists thanks to the support of Highwire Press, Inc and the Institute for the Arts and Humanities at UNC and was created as part of the Symposiac Conference Platform.

## Requirements

- Drupal 9, 10, or 11
- Views (core module)

## Installation

1. Install the module using Composer:
   ```
   composer require drupal/alpha_pagination
   ```
2. Enable the module at Administration > Extend (`/admin/modules`).

## Views Integration and Configuration

1. Build a new view of either users, content (nodes), or comments.

2. Add whatever field you want to use as the basis for the alphabetic grouping (e.g. title, body). You can optionally exclude this field from display if you don't want it to appear in the results shown on the page for some reason. You can only choose a field that is a textfield, textarea or a textarea with a summary.

3. Add either a header or a footer to your view. Select the new item available in the menu of options for **Global: Alpha Pagination**.

4. Configure how you want alpha_pagination to work and specify where it should appear:

   a. Set the path to the results view page.

   b. Select the field you want to use as the basis for the alphabetic grouping from the options presented in the select list. If the field you want to use does not appear, go back and add it to your view and then return to this configuration page to select the field.

   c. Add a contextual filter that is the same as the field you wish to use as the basis for alphabetic sorting. Be sure to enable **Glossary mode** and set the character limit to **1**. The transform case option on the URL should be set to **Upper Case**. Also be sure this is the value item for the field and not something else like formatter.

   d. By default the alpha pagination will apply to all displays; if you only want the alpha pagination to appear on the current display, use the drop-down menu at the top of the administrative interface to change the setting from "All displays" to "This page (override)".

5. An optional sample view is included and can be enabled via the `alpha_pagination_sample_view` submodule. The sample relies on the "article" default content type. You can create sample content using the Devel module or rely on your own data.

## Handling Items With No Results

Letters (and numbers, if enabled) that have no matching content are rendered as inactive items. Instead of clickable links, they appear as plain `<span>` elements with the configured inactive CSS class (default: `inactive`). This lets you visually distinguish letters that have results from those that don't — for example, by graying out or hiding inactive items via CSS.

You can customize this behavior in the Alpha Pagination header/footer area configuration for your view:

- **Inactive item class** — Set the CSS class applied to letters/numbers with no results. Use this to style them differently (e.g., `display: none` to hide them entirely, or reduced opacity to gray them out).
- **Hide all numeric items if empty** — When numeric items are enabled, this checkbox will remove all numeric items from the pagination bar entirely if no content starts with a number. This is useful when your content rarely begins with digits and you don't want empty numeric items cluttering the navigation.

## Additional Styling

You can specify the CSS style classes around the paginator, the item list, and each item whether or not it is active or inactive. This includes the ability to set a class for linked items (as opposed to items which have no options and therefore no links).

You can also add an "All" item as well as numeric items and specify their positions in navigation, the label, and their respective CSS classes.

## Multi-language Support

The module includes built-in alphabets for English, Arabic, and Russian. Additional languages can be added via `hook_alpha_pagination_alphabet_alter()` and `hook_alpha_pagination_numbers_alter()`. See `alpha_pagination.api.php` for details.

## Credits

- [Additional styles & configuration options](https://www.drupal.org/node/2664344) by [MarkCarver](https://www.drupal.org/u/markcarver)
- [Arabic Letter Support](https://www.drupal.org/node/2654274) by [Jad Sayegh](https://www.drupal.org/u/jadsay)
