<?php

/**
 * @file
 * The API for the Views Alpha Pagination module.
 */

/**
 * @defgroup alpha_pagination_api_hooks Alpha Pagination API Hooks
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Allows modules and themes to alter the alphabets array prior to rendering.
 *
 * Note: Do not use range(); always be explicit when defining an alphabet.
 * This is necessary as you cannot rely on the server language to construct
 * proper alphabet characters.
 *
 * @param array $alphabets
 *   An associative array of key/value pairs, passed by reference. Each key is
 *   a corresponding langcode and its value is an indexed array of characters
 *   for that langcode.
 * @param \views_handler_area_alpha_pagination $view
 *   The View instance.
 *
 * @see \views_handler_area_alpha_pagination::getAlphabet()
 * @ingroup alpha_pagination_api_hooks
 */
function hook_alpha_pagination_alphabet_alter(array &$alphabets, \views_handler_area_alpha_pagination $view) {
  // Remove Z from the list.
  array_pop($alphabets['en']);
}

/**
 * Allows modules and themes to alter the numbers array prior to rendering.
 *
 * Note: Do not use range(); always be explicit when defining numbers.
 * This is necessary as you cannot rely on the server language to construct
 * proper numerical characters.
 *
 * @param array $numbers
 *   An associative array of key/value pairs, passed by reference. Each key is
 *   a corresponding langcode and its value is an indexed array of numbers
 *   for that langcode.
 * @param \views_handler_area_alpha_pagination $view
 *   The View instance.
 *
 * @see \views_handler_area_alpha_pagination::getNumbers()
 * @ingroup alpha_pagination_api_hooks
 */
function hook_alpha_pagination_numbers_alter(array &$numbers, \views_handler_area_alpha_pagination $view) {
  // Remove 0 from the list.
  array_shift($numbers['en']);
}

/**
 * @} End of "addtogroup hooks".
 */
