<?php

/**
 * @file
 * Contains alpha_pagination\alpha_pagination.views.inc.
 *
 * Provide a custom views field data that isn't tied to any other module.
 */

/**
 * Implements hook_views_data().
 */
function alpha_pagination_views_data() {

  $data['views']['table']['group'] = t('Custom Global');
  $data['views']['table']['join'] = [
    // #global is a special flag which allows a table to appear all the time.
    '#global' => [],
  ];

  $data['views']['alpha_pagination'] = [
    'title' => t('Alpha Pagination'),
    'help' => t('Adds alphanumeric pagination.'),
    'area' => [
      'id' => 'alpha_pagination',
    ],
  ];

  $data['views']['alpha_pagination_group'] = [
    'title' => t('Alpha Pagination'),
    'help' => t('Adds an automated field that can be used to group by the alphanumeric pagination prefix.'),
    'field' => [
      'id' => 'alpha_pagination_group',
    ],
  ];

  return $data;
}
