<?php

/**
 * @file
 * Set up our views hook data.
 */

/**
 * Implements hook_views_data().
 */
function alpha_pagination_views_data() {
  $data = array();
  $data['views']['alpha_pagination'] = array(
    'title' => t('Alpha Pagination'),
    'group' => 'Global',
    'area' => array(
      'help' => t('Adds alphanumeric pagination.'),
      'handler' => 'views_handler_area_alpha_pagination',
    ),
  );
  $data['views']['alpha_pagination_group'] = array(
    'title' => t('Alpha Pagination'),
    'group' => 'Global',
    'field' => array(
      'help' => t('Adds an automated field that can be used to group by the alphanumeric pagination prefix.'),
      'handler' => 'views_handler_field_alpha_pagination_group',
    ),
  );
  return $data;
}
