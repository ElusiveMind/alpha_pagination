# Changelog — Alpha Pagination (7.x-2.x)

All notable changes to the Drupal 7 branch of the Alpha Pagination module.

## [Unreleased]

- PHP 8.3 compatibility. (2026-03-25)

## [7.x-2.0] — 2017-06-05

Major rewrite introducing an object-oriented architecture, new features, and
numerous bug fixes. This release includes all changes from 7.x-2.0-alpha1.

### New Features

- **Issue [#2883062](https://www.drupal.org/node/2883062)**: Add field handler
  to group by prefix. (markcarver)
- **Issue [#2573073](https://www.drupal.org/node/2573073)**: Better path
  support. (matias, lionslair, markcarver)
- **Issue [#2649396](https://www.drupal.org/node/2649396)**: Support Russian
  alphabet. (markcarver)
- **Issue [#2509946](https://www.drupal.org/node/2509946)**: Provide
  methods/alters for supplying alphanumeric characters via
  `hook_alpha_pagination_alphabet_alter` and
  `hook_alpha_pagination_numbers_alter`. (markcarver)
- **Issue [#2881871](https://www.drupal.org/node/2881871)**: Add more control
  over numeric items. (markcarver)

### Bug Fixes

- **Issue [#2883869](https://www.drupal.org/node/2883869)**: Views handler
  validation is always executed. (markcarver)
- Fix bugs introduced by moving files to the OO structure.
- **Issue [#2881880](https://www.drupal.org/node/2881880)**: Cache identifier
  is not locale specific. (markcarver)
- **Issue [#2876852](https://www.drupal.org/node/2876852)**: Does not work with
  cached views. (Alex Bukach)
- Fix incorrect variable name: use `language` instead of `langcode`.

### Changes

- **Issue [#2883540](https://www.drupal.org/node/2883540)**: Convert
  `pre_letter_path` option to `paginate_link_path`. (markcarver)
- **Issue [#2883107](https://www.drupal.org/node/2883107)**: Set minimum PHP
  requirement to 5.4. (markcarver)
- Organize codebase into OO workflow for parity with the 8.x branch.
- **Issue [#2881873](https://www.drupal.org/node/2881873)**: Update default
  alpha pagination styling. (markcarver)
- Miscellaneous coding standard fixes.

## [7.x-1.6] — 2017-05-23

### Bug Fixes

- **Issue [#2880747](https://www.drupal.org/node/2880747)**: Column not found:
  1054 Unknown column 'title'. (crystaldawn, ElusiveMind)

## [7.x-1.5] — 2017-05-09

### Bug Fixes

- **Issue [#2876852](https://www.drupal.org/node/2876852)**: Does not work with
  cached views. (Alex Bukach)
- **Issue [#2876854](https://www.drupal.org/node/2876854)**: Respect exposed
  filters query. (Alex Bukach)

## [7.x-1.4] — 2017-05-01

### Bug Fixes

- **Issue [#2845135](https://www.drupal.org/node/2845135)**: Wrong letter in
  pager. (ElusiveMind, Rudi Teschner)
- **Issue [#2819849](https://www.drupal.org/node/2819849)**: "All" is
  unconditionally added even when option is disabled. (czycha)
- **Issue [#2824724](https://www.drupal.org/node/2824724)**: The requested page
  could not be found. (pengi, ElusiveMind)
- Fixed the sample view.

### Changes

- Made entity module a dependency.
- Updated readme and provided advanced help documentation.
- General code cleanup.

## [7.x-1.3] — 2016-05-22

### New Features

- **Issue [#2664344](https://www.drupal.org/node/2664344)**: Plugin hardcodes a
  lot of theme-able output and is missing necessary options. (markcarver)
- **Issue [#2654274](https://www.drupal.org/node/2654274)**: Arabic letters
  support. (jadsay)

## [7.x-1.2] — 2015-11-01

### New Features

- **Issue [#2553065](https://www.drupal.org/node/2553065)**: Works with
  Features module. (kristofferwiklund)
- **Issue [#2557999](https://www.drupal.org/node/2557999)**: Numbers support.
  (earelin, ElusiveMind)
- Added CSS support for active and inactive letters/numbers.

### Bug Fixes

- Fixed an issue where an argumentless call to the page would produce errors.

## [7.x-1.1] — 2015-06-02

### New Features

- **Issue [#2496213](https://www.drupal.org/node/2496213)**: Add support for
  Name module, use Drupal APIs, alter HTML structure. (hashmap)

## [7.x-1.0] — 2015-05-21

- Initial release.
