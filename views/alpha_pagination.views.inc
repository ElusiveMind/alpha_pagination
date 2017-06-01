<?php

/**
 * @file
 * Set up our views hook data.
 */

/**
 * Implements hook_views_data().
 */
function alpha_pagination_views_data() {
  $data = [];
  $data['views']['alpha_pagination'] = [
    'title' => t('Alpha Pagination'),
    'group' => 'Global',
    'area' => [
      'help' => t('Adds alphanumeric pagination.'),
      'handler' => 'views_handler_area_alpha_pagination',
    ],
  ];
  $data['views']['alpha_pagination_group'] = [
    'title' => t('Alpha Pagination'),
    'group' => 'Global',
    'field' => [
      'help' => t('Adds an automated field that can be used to group by the alphanumeric pagination prefix.'),
      'handler' => 'views_handler_field_alpha_pagination_group',
    ],
  ];
  return $data;
}
