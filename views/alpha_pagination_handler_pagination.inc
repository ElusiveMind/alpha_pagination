<?php

/**
 * @file
 * Definition of alpha_pagination_handler_pagination.
 */

/**
 * Views area handler to display alphabetic pagination.
 *
 * @ingroup views_area_handlers
 */
class alpha_pagination_handler_pagination extends views_handler_area {

  /**
   * Our option default definitions.
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
      'default' => implode('+', $this->getNumbers()),
      'translatable' => FALSE,
    );

    return $options;
  }

  /**
   * {@inheritdoc}
   */
  function options_submit(&$form, &$form_state) {
    // Need to clear cache when options have changed.
    cache_clear_all($this->getCid(), 'cache', TRUE);

    // Filter attributes for any XSS vulnerabilities before saving.
    if (!empty($form_state['values']['options']['paginate_link_attributes'])) {
      $form_state['values']['options']['paginate_link_attributes'] = filter_xss_admin($form_state['values']['options']['paginate_link_attributes']);
    }
  }

  /**
   * Our options form.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

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

    if (module_exists('token')) {
      $tokens = array(
        '#type' => 'container',
        '#title' => t('Browse available tokens'),
      );
      $tokens['help'] = array(
        '#theme' => 'token_tree_link',
        '#token_types' => array('alpha_pagination'),
        '#global_types' => TRUE,
        '#dialog' => TRUE,
      );
    }
    else {
      $tokens = array(
        '#type' => 'fieldset',
        '#title' => 'Available tokens',
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );
      $items = array();
      $token_info = alpha_pagination_token_info();
      foreach ($token_info['tokens'] as $_type => $_tokens) {
        foreach ($_tokens as $_token => $_data) {
          $items[] = "[$_type:$_token] - {$_data['description']}";
        }
      }
      $tokens['help'] = array(
        '#theme' => 'item_list',
        '#items' => $items,
      );
    }

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

    if ($tokens) {
      $form['paginate_link_tokens'] = $tokens;
      $form['paginate_link_tokens']['#fieldset'] = 'paginate_link';
    }

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
    $this->ensureQuery();
  }

  /**
   * Extract the SQL query from the query information.
   *
   * Once extracted, place it into the options array so it is passed to the
   * render function. This code was lifted nearly verbatim from the views
   * module where the query is constructed for the ui to show the query in the
   * administrative area.
   *
   * @todo Need to make this better somehow?
   */
  function ensureQuery() {
    if (empty($this->options['query']) && !empty($this->view->build_info['query'])) {
      $query = $this->view->build_info['query'];
      $quoted = $query->getArguments();
      $connection = Database::getConnection();
      foreach ($quoted as $key => $val) {
        if (is_array($val)) {
          $quoted[$key] = implode(', ', array_map(array(
            $connection,
            'quote',
          ), $val));
        }
        else {
          $quoted[$key] = $connection->quote($val);
        }
      }
      $this->options['query'] = check_plain(strtr($query, $quoted));
    }
  }

  /**
   * Render the alphabetic pagination.
   *
   * @param bool $empty
   *   If this area should be empty, then return it as empty.
   *
   * @return string $paginator
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
    $this->addOptionClasses('paginate_class', $wrapper['#attributes']);

    $values = $this->getItems();

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
        $item_data = $this->buildLink($label, $value);
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

        $this->addOptionClasses('paginate_numeric_divider_class', $item);

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
            $this->addOptionClasses('paginate_all_class', $item);
          }
          if ($numeric) {
            $this->addOptionClasses('paginate_numeric_class', $item);
          }
          if ($active) {
            $this->addOptionClasses('paginate_active_class', $item);
          }
          elseif (!$is_link) {
            $this->addOptionClasses('paginate_inactive_class', $item);
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
    $this->addOptionClasses('paginate_list_class', $item_list['#attributes']);

    // Append the item list to the wrapper.
    $wrapper[] = $item_list;

    return drupal_render($wrapper);
  }

  /**
   * Add classes to an attributes array from a view option.
   *
   * @param string $option
   *   The name of the view option that contains a space separated list of
   *   classes.
   * @param array $attributes
   *   An attributes array to add the classes to, passed by reference.
   *
   * @return array
   *   An array of classes to be used in a render array.
   */
  protected function addOptionClasses($option, array &$attributes) {
    // Sanitize any classes provided for the item list.
    $classes = array_filter(explode(' ', $this->options[$option]));
    foreach ($classes as &$class) {
      $class = views_clean_css_identifier($class);
    }

    // Don't add any classes if it's empty, which will add an empty attribute.
    if ($classes) {
      if (!isset($attributes['class'])) {
        $attributes['class'] = array();
      }
      $attributes['class'] = array_unique(array_merge($attributes['class'], $classes));
    }

    return $classes;
  }

  /**
   * Builds a link render array based on current text, value and options.
   *
   * @param string $text
   *   The current text label.
   * @param string $value
   *   The current value.
   * @param array $options
   *   Optional. Array of options to pass to url().
   *
   * @return array
   *   A render array for the link.
   */
  protected function buildLink($text, $value, array $options = array()) {
    // Merge in options.
    $options = drupal_array_merge_deep(array(
      'attributes' => array(),
      'html' => FALSE,
      'query' => drupal_get_query_parameters(),
    ), $options);

    // Merge in classes.
    $this->addOptionClasses('paginate_link_class', $options['attributes']);

    $path = token_replace($this->options['paginate_link_path'], $this->getTokens($value));

    // Determine if link is external (automatically enforcing for anchors).
    if ($this->options['paginate_link_external'] || ($path && $path[0] === '#')) {
      $options['external'] = TRUE;
    }

    // Add in additional attributes.
    if ($this->options['paginate_link_attributes']) {
      $attributes = $this->parseAttributes($this->options['paginate_link_attributes'], $this->getTokens($value));
      // Remove any class attributes (should use the dedicated class option).
      unset($attributes['class']);

      // Merge in the attributes.
      $options['attributes'] = drupal_array_merge_deep($options['attributes'], $attributes);
    }

    // Build link render array.
    return array(
      '#theme' => 'link__alpha_pagination',
      '#text' => $text,
      '#path' => $path,
      '#options' => $options,
    );
  }

  /**
   * Retrieves alphabet characters, based on langcode.
   *
   * Note: Do not use range(); always be explicit when defining an alphabet.
   * This is necessary as you cannot rely on the server language to construct
   * proper alphabet characters.
   *
   * @param string $langcode
   *   The langcode to return. If the langcode does not exist, it will default
   *   to English.
   *
   * @return array
   *   An indexed array of alphabet characters, based on langcode.
   *
   * @see hook_alpha_pagination_alphabet_alter()
   */
  protected function getAlphabet($langcode = NULL) {
    global $language;

    // Default (English).
    static $default = array('A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z');
    static $alphabets;

    // If the langcode is not explicitly specified, default to global langcode.
    if (!isset($langcode)) {
      $langcode = $language->language;
    }

    // Retrieve alphabets.
    if (!isset($alphabets)) {
      // Attempt to retrieve from database cache.
      $cid = "alpha_pagination:alphabets";
      if (($cache = cache_get($cid)) && !empty($cache->data)) {
        $alphabets = $cache->data;
      }
      // Build alphabets.
      else {
        // Arabic.
        $alphabets['ar'] = array('ا', 'ب', 'ت', 'ث', 'ج', 'ح', 'خ', 'د', 'ذ', 'ر', 'ز', 'س', 'ش', 'ص', 'ض', 'ط', 'ظ', 'ع', 'غ', 'ف', 'ق', 'ك', 'ل', 'م', 'ن', 'و', 'ه', 'ي');

        // English. Initially the default value, but can be modified in alter.
        $alphabets['en'] = $default;

        // Русский (Russian).
        $alphabets['ru'] = array('А', 'Б', 'В', 'Г', 'Д', 'Е', 'Ё', 'Ж', 'З', 'И', 'Й', 'К', 'Л', 'М', 'Н', 'О', 'П', 'Р', 'С', 'Т', 'У', 'Ф', 'Х', 'Ц', 'Ч', 'Ш', 'Щ', 'Ы', 'Э', 'Ю', 'Я');

        // Allow modules and themes to alter alphabets.
        drupal_alter('alpha_pagination_alphabet', $alphabets, $this);

        // Cache the alphabets.
        cache_set($cid, $alphabets);
      }
    }

    // Return alphabet based on langcode.
    return isset($alphabets[$langcode]) ? $alphabets[$langcode] : $default;
  }

  /**
   * Retrieves a cache identifier for the view, display and query, if set.
   *
   * @return string
   *   A cache identifier.
   */
  protected function getCid() {
    global $language;
    $this->ensureQuery();
    $query = !empty($this->options['query']) ? md5($this->options['query']) : '';
    return "alpha_pagination:{$language->language}:{$this->view->name}:{$this->view->current_display}:$query";
  }

  /**
   * Retrieves numeric characters, based on langcode.
   *
   * Note: Do not use range(); always be explicit when defining numbers.
   * This is necessary as you cannot rely on the server language to construct
   * proper numeric characters.
   *
   * @param string $langcode
   *   The langcode to return. If the langcode does not exist, it will default
   *   to English.
   *
   * @return array
   *   An indexed array of numeric characters, based on langcode.
   *
   * @see hook_alpha_pagination_numbers_alter()
   */
  protected function getNumbers($langcode = NULL) {
    global $language;

    // Default (English).
    static $default = array('0', '1', '2', '3', '4', '5', '6', '7', '8', '9');
    static $numbers;

    // If the langcode is not explicitly specified, default to global langcode.
    if (!isset($langcode)) {
      $langcode = $language->language;
    }

    // Retrieve numbers.
    if (!isset($numbers)) {
      // Attempt to retrieve from database cache.
      $cid = "alpha_pagination:numbers";
      if (($cache = cache_get($cid)) && !empty($cache->data)) {
        $numbers = $cache->data;
      }
      // Build numbers.
      else {
        // English. Initially the default value, but can be modified in alter.
        $numbers['en'] = $default;

        // Allow modules and themes to alter numbers.
        drupal_alter('alpha_pagination_numbers', $numbers, $this);

        // Cache the numbers.
        cache_set($cid, $numbers);
      }
    }

    // Return numbers based on langcode.
    return isset($numbers[$langcode]) ? $numbers[$langcode] : $default;
  }

  /**
   * Retrieves the items used to populate the pagination item list.
   *
   * @return array
   *   An associative array containing the result state as the value, keyed by
   *   the letter, number or "all".
   */
  protected function getItems() {

    // Check to see if this query is cached. If it is, then just pull our
    // results set from it so that things can move quickly through here. We're
    // caching in the event the view has a very large result set.
    $cid = $this->getCid();
    if (($cache = cache_get($cid)) && !empty($cache->data)) {
      return $cache->data;
    }

    // Retrieve the alphabet characters.
    $items = $this->getAlphabet();

    // Append or prepend numeric items.
    $numbers = $this->options['paginate_view_numbers'] == 2 ? array(check_plain($this->options['paginate_numeric_label'])) : $this->getNumbers();
    if (!empty($this->options['paginate_view_numbers'])) {
      $numeric_divider = $this->options['paginate_numeric_divider'] ? array('-') : array();
      if ($this->options['paginate_numeric_position'] === 'after') {
        $items = array_merge($items, $numeric_divider, $numbers);
      }
      else {
        $items = array_merge($numbers, $numeric_divider, $items);
      }
    }

    // Append or prepend the "all" item.
    if (!empty($this->options['paginate_all_display'])) {
      if ($this->options['paginate_all_position'] === 'before') {
        $items = array_merge(array('all'), $items);
      }
      else {
        $items[] = 'all';
      }
    }

    // Initialize the results with FALSE values so each prefix is disabled by
    // default. They're later filled in as TRUE below when there is actual
    // entity prefixes that exist.
    $results = array_fill_keys($items, FALSE);

    // Retrieve the entity prefixes.
    if ($prefixes = $this->getEntityPrefixes()) {
      // Ensure that "all" is TRUE if there are prefixes.
      if (isset($results['all'])) {
        $results['all'] = TRUE;
      }

      // Set prefixes to TRUE if it exists from the default list that was
      // constructed from view options above.
      foreach ($prefixes as $prefix) {
        if (is_numeric($prefix) && $this->options['paginate_view_numbers'] == 2) {
          $prefix = $this->options['paginate_numeric_label'];
        }
        if (isset($results[$prefix])) {
          $results[$prefix] = TRUE;
        }
      }
    }

    // Remove all numeric values if they're all empty.
    if ($this->options['paginate_numeric_hide_empty']) {
      // Determine if numeric results are not empty.
      $numeric_results = array_filter(array_intersect_key($results, array_flip($numbers)));
      if (!$numeric_results) {
        $results = array_diff_key($results, array_flip($numbers));
      }
    }

    // Cache the results.
    cache_set($cid, $results, 'cache');
    return $results;
  }

  /**
   * Retrieve the distinct first character prefix from the field tables.
   *
   * Mark them as TRUE so their pagination item is represented properly.
   *
   * Note that the node title is a special case that we have to take from the
   * node table as opposed to the body or any custom fields.
   *
   * @todo This should be cleaned up more and fixed "properly".
   *
   * @return array
   *   An indexed array containing a unique array of entity prefixes.
   */
  protected function getEntityPrefixes() {
    $prefixes = array();

    if ($entity_ids = $this->getEntityIds()) {
      switch ($this->options['paginate_view_field']) {
        case 'name':
          $table = $this->view->base_table;
          $where = $this->view->base_field;

          // Extract the "name" field from the entity property info.
          $table_data = views_fetch_data($table);
          $entity_info = entity_get_property_info($table_data['table']['entity type']);
          $field = isset($entity_info['properties']['name']['schema field']) ? $entity_info['properties']['name']['schema field'] : 'name';
          break;

        case 'title':
          $table = $this->view->base_table;
          $where = $this->view->base_field;

          // Extract the "title" field from the entity property info.
          $table_data = views_fetch_data($table);
          $entity_info = entity_get_property_info($table_data['table']['entity type']);
          $field = isset($entity_info['properties']['title']['schema field']) ? $entity_info['properties']['title']['schema field'] : 'title';
          break;

        default:
          if (strpos($this->options['paginate_view_field'], ':') === FALSE) {
            // Format field name and table for single value fields
            $field = $this->options['paginate_view_field'] . '_value';
            $table = 'field_data_' . $this->options['paginate_view_field'];
          }
          else {
            // Format field name and table for compound value fields
            $field = str_replace(':', '_', $this->options['paginate_view_field']);
            $field_name_components = explode(':', $this->options['paginate_view_field']);
            $table = 'field_data_' . $field_name_components[0];
          }
          $where = 'entity_id';
          break;
      }
      $result = db_query('SELECT DISTINCT(SUBSTR(' . $field . ', 1, 1)) AS prefix
                          FROM {' . $table . '}
                          WHERE ' . $where . ' IN ( :nids )', array(':nids' => $entity_ids));
      while ($data = $result->fetchObject()) {
        $prefixes[] = is_numeric($data->prefix) ? $data->prefix : drupal_strtoupper($data->prefix);
      }
    }
    return array_unique(array_filter($prefixes));
  }

  /**
   * Construct the actual SQL query for the view being generated.
   *
   * Then parse it to short-circuit certain conditions that may exist and
   * make any alterations. This is not the most elegant of solutions, but it
   * is very effective.
   *
   * @return array
   *   An indexed array of entity identifiers.
   */
  protected function getEntityIds() {
    $this->ensureQuery();
    $query_parts = explode("\n", $this->options['query']);

    // Get the base field. This will change depending on the type of view we
    // are putting the paginator on.
    $base_field = $this->view->base_field;

    // If we are dealing with a substring, then short circuit it as we are most
    // likely dealing with a glossary contextual filter.
    foreach ($query_parts as $k => $part) {
      if ($position = strpos($part, "SUBSTRING")) {
        $part = substr($part, 0, $position) . " 1 OR " . substr($part, $position);
        $query_parts[$k] = $part;
      }
    }

    // Evaluate the last line looking for anything which may limit the result
    // set as we need results against the entire set of data and not just what
    // is configured in the view.
    $last_line = array_pop($query_parts);
    if (substr($last_line, 0, 5) != "LIMIT") {
      $query_parts[] = $last_line;
    }

    // Construct the query from the array and change the single quotes from
    // HTML special characters back into single quotes.
    $query = join("\n", $query_parts);
    $query = str_replace("&#039;", '\'', $query);
    $query = str_replace("&amp;", '&', $query);
    $query = str_replace("&lt;", '<', $query);
    $query = str_replace("&gt;", '>', $query);

    // Based on our query, get the list of entity identifiers that are affected.
    // These will be used to generate the pagination items.
    $entity_ids = array();
    $result = db_query($query);
    while ($data = $result->fetchObject()) {
      $entity_ids[] = $data->$base_field;
    }
    return $entity_ids;
  }

  /**
   * Provides the token data that is passed when during replacement.
   *
   * @param string $value
   *   The current character value being processed.
   *
   * @return array
   *   Token data.
   *
   * @see alpha_pagination_token_info()
   * @see alpha_pagination_tokens()
   */
  protected function getTokens($value = NULL) {
    return array(
      'alpha_pagination' => array(
        'path' => $this->getUrl(),
        'value' => $value,
      ),
    );
  }

  /**
   * Retrieves the URL for the current view.
   *
   * Note: this follows very similarly to \view::get_url to process arguments,
   * however it is in fact severely modified to account for characters appended
   * by this module.
   *
   * @return string
   *   The URL for the view or current_path().
   */
  protected function getUrl() {
    static $url;

    if (!isset($url)) {
      if (!empty($this->view->override_url)) {
        return $this->view->override_url;
      }

      $path = $this->view->get_path();
      $args = $this->view->args;

      // Exclude arguments that were computed, not passed on the URL.
      $position = 0;
      if (!empty($this->argument)) {
        foreach ($this->argument as $argument_id => $argument) {
          if (!empty($argument->options['default_argument_skip_url'])) {
            unset($args[$position]);
          }
          $position++;
        }
      }

      // Don't bother working if there's nothing to do:
      if (empty($path) || (empty($args) && strpos($path, '%') === FALSE)) {
        $path = current_path();
        $pieces = explode('/', $path);
        if (array_key_exists(end($pieces), $this->getItems())) {
          array_pop($pieces);
        }
        $url = implode('/', $pieces);
        return $url;
      }

      $pieces = array();
      $argument_keys = isset($this->argument) ? array_keys($this->argument) : array();
      $id = current($argument_keys);
      foreach (explode('/', $path) as $piece) {
        if ($piece != '%') {
          $pieces[] = $piece;
        }
        else {
          if (empty($args)) {
            // Try to never put % in a url; use the wildcard instead.
            if ($id && !empty($this->argument[$id]->options['exception']['value'])) {
              $pieces[] = $this->argument[$id]->options['exception']['value'];
            }
            else {
              $pieces[] = '*'; // gotta put something if there just isn't one.
            }

          }
          else {
            $pieces[] = array_shift($args);
          }

          if ($id) {
            $id = next($argument_keys);
          }
        }
      }

      // Just return the computed pieces, don't merge any extra remaining args.
      $url = implode('/', $pieces);
    }
    return $url;
  }

  /**
   * Parses an attribute string saved in the UI.
   *
   * @param string $string
   *   The attribute string to parse.
   * @param array $tokens
   *   An associative array of token data to use.
   *
   * @return array
   *   A parsed attributes array.
   */
  protected function parseAttributes($string = NULL, array $tokens = []) {
    $attributes = [];
    if (!empty($string)) {
      $parts = explode(',', $string);
      foreach ($parts as $attribute) {
        if (strpos($attribute, '|') !== FALSE) {
          list($key, $value) = explode('|', token_replace($attribute, $tokens, ['clear' => TRUE]));
          $attributes[$key] = $value;
        }
      }
    }
    return $attributes;
  }

}
