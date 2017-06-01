<?php

/**
 * Views area handler to display alphabetic pagination.
 *
 * @ingroup views_area_handlers
 */
class views_handler_area_alpha_pagination extends \views_handler_area {

  /**
   * @var \AlphaPaginationHelper
   */
  protected $helper;

  /**
   * {@inheritdoc}
   */
  public function construct() {
    $this->helper = new AlphaPaginationHelper($this);
    parent::construct();
  }

  /**
   * {@inheritdoc}
   */
  public function label() {
    return '';
  }

  /**
   * {@inheritdoc}
   */
  function option_definition() {
    $options = parent::option_definition();

    // Global.
    $options['paginate_view_field'] = array(
      'default' => 'title',
      'translatable' => TRUE,
    );

    // Link.
    $options['paginate_link_path'] = array(
      'default' => '[alpha_pagination:path]/[alpha_pagination:value]',
      'translatable' => FALSE,
    );
    $options['paginate_link_external'] = array(
      'default' => 0,
      'translatable' => FALSE,
    );
    $options['paginate_link_class'] = array(
      'default' => '',
      'translatable' => FALSE,
    );
    $options['paginate_link_attributes'] = array(
      'default' => '',
      'translatable' => FALSE,
    );

    // Classes.
    $options['paginate_class'] = array(
      'default' => 'alpha-pagination',
      'translatable' => FALSE,
    );
    $options['paginate_list_class'] = array(
      'default' => 'alpha-pagination-list',
      'translatable' => FALSE,
    );
    $options['paginate_active_class'] = array(
      'default' => 'active',
      'translatable' => FALSE,
    );
    $options['paginate_inactive_class'] = array(
      'default' => 'inactive',
      'translatable' => FALSE,
    );

    // All.
    $options['paginate_all_display'] = array(
      'default' => 1,
      'translatable' => FALSE,
    );
    $options['paginate_all_class'] = array(
      'default' => 'all',
      'translatable' => FALSE,
    );
    $options['paginate_all_label'] = array(
      'default' => t('All'),
      'translatable' => TRUE,
    );
    $options['paginate_all_position'] = array(
      'default' => 'after',
      'translatable' => FALSE,
    );
    $options['paginate_toggle_empty'] = array(
      'default' => 1,
      'translatable' => FALSE,
    );

    // Numeric.
    $options['paginate_view_numbers'] = array(
      'default' => 0,
      'translatable' => FALSE,
    );
    $options['paginate_numeric_class'] = array(
      'default' => 'numeric',
      'translatable' => FALSE,
    );
    $options['paginate_numeric_divider'] = array(
      'default' => 1,
      'translatable' => FALSE,
    );
    $options['paginate_numeric_divider_class'] = array(
      'default' => 'numeric-divider',
      'translatable' => FALSE,
    );
    $options['paginate_numeric_hide_empty'] = array(
      'default' => 1,
      'translatable' => FALSE,
    );
    $options['paginate_numeric_label'] = array(
      'default' => '#',
      'translatable' => FALSE,
    );
    $options['paginate_numeric_position'] = array(
      'default' => 'before',
      'translatable' => FALSE,
    );
    $options['paginate_numeric_value'] = array(
      'default' => implode('+', $this->helper->getNumbers()),
      'translatable' => FALSE,
    );

    return $options;
  }

  /**
   * {@inheritdoc}
   */
  function options_submit(&$form, &$form_state) {
    // Need to clear cache when options have changed.
    cache_clear_all($this->helper->getCid(), 'cache', TRUE);

    // Filter attributes for any XSS vulnerabilities before saving.
    if (!empty($form_state['values']['options']['paginate_link_attributes'])) {
      $form_state['values']['options']['paginate_link_attributes'] = filter_xss_admin($form_state['values']['options']['paginate_link_attributes']);
    }
  }

  /**
   * {@inheritdoc}
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // Hide unnecessary label.
    $form['label']['#access'] = FALSE;

    // Global options.
    if ($this->view->base_table == 'taxonomy_term_data') {
      // Get an array list of all non-image, non-entity or other assorted reference fields.
      $fields = array('name' => 'name');
    }
    else {
      // Get an array list of all non-image, non-entity or other assorted reference fields.
      $fields = array('title' => 'title');
    }

    $compound_field_types = array('name');
    $single_field_types = array('text', 'text_long', 'text_with_summary');
    $all_field_types = array_merge($single_field_types, $compound_field_types);
    foreach (field_info_field_map() as $field_name => $field_definition) {
      if (in_array($field_definition['type'], $all_field_types)) {
        if (in_array($field_definition['type'], $compound_field_types)) {
          $field_info = field_info_field($field_name);
          foreach (array_keys($field_info['columns']) as $compoundFieldKey) {
            $compound_field_field_name = sprintf('%s:%s', $field_name, $compoundFieldKey);
            $fields[$compound_field_field_name] = $compound_field_field_name;
          }
        }
        else {
          $fields[$field_name] = $field_name;
        }
      }
    }
    $form['paginate_view_field'] = array(
      '#title' => t('View field to paginate against'),
      '#type' => 'select',
      '#options' => $fields,
      '#default_value' => $this->options['paginate_view_field'],
      '#description' => t('This will be the content field that drives the pagination.'),
    );

    $form['paginate_toggle_empty'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show options without results'),
      '#default_value' => $this->options['paginate_toggle_empty'],
      '#description' => t('Show or hide letters without results'),
    );

    // Link.
    $form['paginate_link'] = array(
      '#type' => 'fieldset',
      '#title' => t('Link'),
      '#collapsible' => TRUE,
    );

    $form['paginate_link_path'] = array(
      '#title' => t('Path'),
      '#type' => 'textfield',
      '#size' => 60,
      '#default_value' => $this->options['paginate_link_path'],
      '#description' => t('This is the path the link will be rendered with. No beginning or ending slashes.'),
      '#fieldset' => 'paginate_link',
    );

    $form['paginate_link_external'] = array(
      '#type' => 'checkbox',
      '#title' => t('External'),
      '#default_value' => $this->options['paginate_link_external'],
      '#description' => t('Indicates whether this is an external link (not processed). If the above path starts with a hash symbol (#), then this option will automatically enable so it can render as a relative link to an anchor on the current page.'),
      '#fieldset' => 'paginate_link',
    );

    $form['paginate_link_class'] = array(
      '#title' => t('Classes'),
      '#type' => 'textfield',
      '#default_value' => $this->options['paginate_link_class'],
      '#description' => t('CSS classes for the link, separated by spaces.'),
      '#fieldset' => 'paginate_link',
    );

    $form['paginate_link_attributes'] = array(
      '#type' => 'textfield',
      '#title' => t('Attributes'),
      '#description' => 'E.g. id|custom-id,role|navigation,data-key|value',
      '#default_value' => $this->options['paginate_link_attributes'],
      '#fieldset' => 'paginate_link',
    );

    $form['paginate_link_tokens'] = $this->helper->buildTokenTree('paginate_link');

    // Class options.
    $form['paginate_classes'] = array(
      '#type' => 'fieldset',
      '#title' => t('Classes'),
      '#description' => t('Provide additional CSS classes on elements in the pagination; separated by spaces.'),
      '#collapsible' => TRUE,
    );
    $form['paginate_class'] = array(
      '#title' => t('Wrapper'),
      '#type' => 'textfield',
      '#default_value' => $this->options['paginate_class'],
      '#description' => t('The wrapper around the item list.'),
      '#fieldset' => 'paginate_classes',
    );
    $form['paginate_list_class'] = array(
      '#title' => t('Item List'),
      '#type' => 'textfield',
      '#default_value' => $this->options['paginate_list_class'],
      '#description' => t('The item list.'),
      '#fieldset' => 'paginate_classes',
    );
    $form['paginate_active_class'] = array(
      '#title' => t('Active item'),
      '#type' => 'textfield',
      '#default_value' => $this->options['paginate_active_class'],
      '#description' => t('The active list item.'),
      '#fieldset' => 'paginate_classes',
    );
    $form['paginate_inactive_class'] = array(
      '#title' => t('Inactive item'),
      '#type' => 'textfield',
      '#default_value' => $this->options['paginate_inactive_class'],
      '#description' => t('The inactive list item(s) that are not links, a.k.a. "no results".'),
      '#fieldset' => 'paginate_classes',
    );

    // "All" options.
    $form['paginate_all_options'] = array(
      '#type' => 'fieldset',
      '#title' => t('"All" item'),
      '#collapsible' => TRUE,
    );
    $form['paginate_all_display'] = array(
      '#type' => 'select',
      '#title' => t('Display the "All" item'),
      '#options' => array(
        0 => t('No'),
        1 => t('Yes'),
      ),
      '#default_value' => $this->options['paginate_all_display'],
      '#description' => t('Displays the "All" link in the pagination.'),
      '#fieldset' => 'paginate_all_options',
    );
    $form['paginate_all_position'] = array(
      '#type' => 'select',
      '#title' => t('Position'),
      '#options' => array(
        'before' => t('Before'),
        'after' => t('After'),
      ),
      '#default_value' => $this->options['paginate_all_position'],
      '#description' => t('Determines where the "All" item will show up in the pagination.'),
      '#dependency' => array('edit-options-paginate-all-display' => array(1)),
      '#fieldset' => 'paginate_all_options',
    );
    $form['paginate_all_label'] = array(
      '#type' => 'textfield',
      '#title' => t('Label'),
      '#default_value' => $this->options['paginate_all_label'],
      '#description' => t('The label to use for display the "All" item in the pagination.'),
      '#dependency' => array('edit-options-paginate-all-display' => array(1)),
      '#fieldset' => 'paginate_all_options',
    );
    $form['paginate_all_class'] = array(
      '#title' => t('Classes'),
      '#type' => 'textfield',
      '#default_value' => $this->options['paginate_all_class'],
      '#description' => t('CSS classes for "All" item (on <code>&lt;li&gt;</code> element); separated by spaces.'),
      '#dependency' => array('edit-options-paginate-all-display' => array(1)),
      '#fieldset' => 'paginate_all_options',
    );

    // "Numeric" options.
    $form['paginate_numeric_options'] = array(
      '#type' => 'fieldset',
      '#title' => t('Numeric items'),
      '#collapsible' => TRUE,
    );

    $form['paginate_view_numbers'] = array(
      '#title' => t('Display numeric items'),
      '#type' => 'select',
      '#options' => array(
        0 => t('No'),
        1 => t('Individual numbers (0-9)'),
        2 => t('Single label (#)'),
      ),
      '#default_value' => $this->options['paginate_view_numbers'],
      '#description' => t('Displays numeric item(s) in the pagination.'),
      '#fieldset' => 'paginate_numeric_options',
    );

    // Global numeric options.
    $form['paginate_numeric_class'] = array(
      '#title' => t('Classes'),
      '#type' => 'textfield',
      '#default_value' => $this->options['paginate_numeric_class'],
      '#description' => t('CSS classes for numeric item (on <code>&lt;li&gt;</code> element); separated by spaces.'),
      '#fieldset' => 'paginate_numeric_options',
      '#dependency' => array(
        'edit-options-paginate-view-numbers' => array(1, 2),
      ),
    );

    $form['paginate_numeric_position'] = array(
      '#type' => 'select',
      '#title' => t('Position'),
      '#options' => array(
        'before' => t('Before'),
        'after' => t('After'),
      ),
      '#default_value' => $this->options['paginate_numeric_position'],
      '#description' => t('Determines whether numeric items are shown before or after alphabetical links in the pagination.'),
      '#fieldset' => 'paginate_numeric_options',
      '#dependency' => array(
        'edit-options-paginate-view-numbers' => array(1, 2),
      ),
    );

    $form['paginate_numeric_hide_empty'] = array(
      '#title' => t('Hide all numeric item(s) if empty'),
      '#description' => t('Will not render any numeric item(s) if there are no results that start with numeric values.'),
      '#type' => 'checkbox',
      '#default_value' => $this->options['paginate_numeric_hide_empty'],
      '#fieldset' => 'paginate_numeric_options',
      '#dependency' => array(
        'edit-options-paginate-view-numbers' => array(1, 2),
      ),
    );

    // Individual numeric items.
    $form['paginate_numeric_divider'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show divider'),
      '#default_value' => $this->options['paginate_numeric_divider'],
      '#description' => t('Will render a specific divider item before or after the numeric items have been render, based on position.'),
      '#fieldset' => 'paginate_numeric_options',
      '#dependency' => array(
        'edit-options-paginate-view-numbers' => array(1),
      ),
    );

    $form['paginate_numeric_divider_class'] = array(
      '#title' => t('Divider class'),
      '#type' => 'textfield',
      '#default_value' => $this->options['paginate_numeric_divider_class'],
      '#description' => t('The class to use for the numeric divider list item.'),
      '#fieldset' => 'paginate_numeric_options',
      '#dependency' => array(
        'edit-options-paginate-view-numbers' => array(1),
        'edit-options-paginate-numeric-divider' => array(1),
      ),
      '#dependency_count' => 2,
    );

    // Single numeric item.
    $form['paginate_numeric_label'] = array(
      '#title' => t('Label'),
      '#type' => 'textfield',
      '#default_value' => $this->options['paginate_numeric_label'],
      '#description' => t('The label to use to represent all numeric values.'),
      '#fieldset' => 'paginate_numeric_options',
      '#dependency' => array('edit-options-paginate-view-numbers' => array(2)),
    );

    $form['paginate_numeric_value'] = array(
      '#title' => t('Value'),
      '#type' => 'textfield',
      '#default_value' => $this->options['paginate_numeric_value'],
      '#description' => t('The value to use to represent all numeric values (i.e. URL value).'),
      '#fieldset' => 'paginate_numeric_options',
      '#dependency' => array('edit-options-paginate-view-numbers' => array(2)),
    );

  }

  /**
   * {@inheritdoc}
   */
  function post_execute(&$values) {
    $this->helper->ensureQuery();
  }

  /**
   * Render the alphabetic pagination.
   *
   * @param bool $empty
   *   If this area should be empty, then return it as empty.
   *
   * @return string
   *   A string representing the complete pagination including linked and
   *   unlinked options.
   */
  function render($empty = FALSE) {
    // Create the wrapper.
    $wrapper = array(
      '#theme_wrappers' => array('container__alpha_pagination__wrapper'),
      '#attributes' => array(),
      '#attached' => array(
        'library' => array(
          array('alpha_pagination', 'alpha_pagination'),
        ),
      ),
    );
    $this->helper->addClasses($this->helper->getOption('paginate_class'), $wrapper['#attributes']);

    $values = $this->helper->getItems();

    // Default to "all" unless a valid argument was provided.
    $current = 'all';

    // Attempt to determine if a valid argument was provided.
    $arg_count = count($this->view->args);
    if ($arg_count) {
      $arg = (string) $this->view->args[$arg_count - 1];
      if ($arg && array_key_exists($arg, $values)) {
        $current = $arg;
      }
    }

    $all_label = !empty($this->options['paginate_all_label']) ? t(check_plain($this->options['paginate_all_label'])) : t('All');

    // Iterate over the alphabet and populate the items for the item list.
    $items = array();
    foreach ($values as $value => $is_link) {
      // All.
      $all = drupal_strtolower($value) === 'all';

      // Numeric.
      $numeric_label = $this->options['paginate_numeric_label'];
      $numeric_value = $this->options['paginate_numeric_value'];
      $single_numeric_label = $this->options['paginate_view_numbers'] == 2;
      $numeric = is_numeric($value) || ($single_numeric_label && $value === $numeric_label);
      $numeric_divider = FALSE;
      if ($value === '-') {
        if (!$single_numeric_label && $this->options['paginate_numeric_divider']) {
          $numeric_divider = TRUE;
          $value = '';
        }
        else {
          continue;
        }
      }

      // Active.
      if ($numeric && $single_numeric_label) {
        $active = (string) $numeric_value === $current;
      }
      else {
        $active = (string) $value === $current;
      }

      // Link.
      $is_link = !$active && ($all || $is_link);

      // Label.
      $label = $all ? $all_label : drupal_ucfirst($value);

      // Ensure a single numeric label alters the URL value.
      if ($single_numeric_label && $value === $this->options['paginate_numeric_label']) {
        $value = $this->options['paginate_numeric_value'];
      }

      // Theme link item.
      if ($is_link) {
        $item_data = $this->helper->buildLink($label, $value);
      }
      // Theme non-link item (a.k.a "inactive").
      //
      // Some themes require text that's in a "pagination" list to be wrapped
      // in another element. Typically, this would be a link, like above,
      // however since this is an inactive option, it should be wrapped in a
      // themeable element that can be targeted by preprocessors if needed.
      elseif ($this->options['paginate_toggle_empty'] || $all || $active) {
        $item_data = array(
          '#type' => 'html_tag',
          '#theme' => 'html_tag__alpha_pagination__inactive',
          '#tag' => 'span',
          '#value' => $label,
        );
      }

      // Add special numeric divider class.
      if ($numeric_divider) {
        // Add an empty list item.
        $item = array('data' => '');

        $this->helper->addClasses($this->helper->getOption('paginate_numeric_divider_class'), $item);

        // Add the constructed item to the list.
        $items[] = $item;
      }
      else {
        if (!empty($item_data)) {
          // Unfortunately, the implementation of item_list in D7 core does not
          // allow render arrays to be passed as data and requires premature
          // rendering here.
          // @todo In D8, pass the render array directly since it can process it.
          $item = array('data' => drupal_render($item_data));

          // Add the necessary classes for item.
          if ($all) {
            $this->helper->addClasses($this->helper->getOption('paginate_all_class'), $item);
          }
          if ($numeric) {
            $this->helper->addClasses($this->helper->getOption('paginate_numeric_class'), $item);
          }
          if ($active) {
            $this->helper->addClasses($this->helper->getOption('paginate_active_class'), $item);
          }
          elseif (!$is_link) {
            $this->helper->addClasses($this->helper->getOption('paginate_inactive_class'), $item);
          }

          // Add the constructed item to the list.
          $items[] = $item;
        }
      }
    }

    // Sanitize any classes provided for the item list.
    $item_list = array(
      '#theme' => 'item_list__alpha_pagination',
      '#attributes' => array(),
      '#items' => $items,
    );
    $this->helper->addClasses($this->helper->getOption('paginate_list_class'), $item_list['#attributes']);

    // Append the item list to the wrapper.
    $wrapper[] = $item_list;

    return drupal_render($wrapper);
  }

  /**
   * {@inheritdoc}
   */
  public function ui_name($short = FALSE) {
    return $this->helper->ui_name();
  }

  /**
   * {@inheritdoc}
   */
  function validate() {
    return $this->helper->validate();
  }

}
