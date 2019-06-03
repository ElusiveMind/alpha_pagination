<?php

namespace Drupal\alpha_pagination\Plugin\views\area;

use Drupal\alpha_pagination\AlphaPagination;
use Drupal\Component\Utility\Xss;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Entity\EntityFieldManagerInterface;
use Drupal\Core\Cache\CacheBackendInterface;
use Drupal\views\Plugin\views\area\AreaPluginBase;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Views area handler to display alphabetic pagination.
 *
 * @ingroup views_area_handlers
 *
 * @ViewsArea("alpha_pagination")
 */
class AlphaPaginationArea extends AreaPluginBase {

  /**
   * The AlphaPagination object reference.
   *
   * @var \Drupal\alpha_pagination\AlphaPagination
   */
  protected $alphaPagination;
  /**
   * The Entity Field Manager Interface.
   *
   * @var \Drupal\Core\Entity\EntityFieldManagerInterface
   */

  protected $fieldManager;

  /**
   * The cache backend service.
   *
   * @var \Drupal\Core\Cache\CacheBackendInterface
   */
  protected $cacheBackend;

  /**
   * Constructs a new RenderedEntity object.
   *
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param array $plugin_definition
   *   The plugin implementation definition.
   * @param \alpha_pagination\AlphaPagination $alpha_pagination
   *   The Alpha Pagination service.
   * @param \Drupal\Core\Entity\EntityFieldManagerInterface $field_manager
   *   The EntityField Manager.
   * @param \Drupal\Core\Cache\CacheBackendInterface $cache_backend
   *   The cache backend service.
   */
  public function __construct(array $configuration, $plugin_id, array $plugin_definition, AlphaPagination $alpha_pagination, EntityFieldManagerInterface $field_manager, CacheBackendInterface $cache_backend) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->alphaPagination = $alpha_pagination;
    $this->alphaPagination->setHandler($this);
    $this->fieldManager = $field_manager;
    $this->cacheBackend = $cache_backend;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static($configuration, $plugin_id, $plugin_definition, $container
      ->get('alpha_pagination'), $container
      ->get('entity_field.manager'), $container
      ->get('cache.default'));
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
  protected function defineOptions() {
    $options = parent::defineOptions();

    // Global.
    $options['paginate_view_relationship'] = [
      'default' => 'none',
      'translatable' => TRUE,
    ];

    $options['paginate_view_field'] = [
      'default' => 'title',
      'translatable' => TRUE,
    ];

    // Link.
    $options['paginate_link_path'] = [
      'default' => '[alpha_pagination:path]/[alpha_pagination:value]',
      'translatable' => FALSE,
    ];
    $options['paginate_link_external'] = [
      'default' => 0,
      'translatable' => FALSE,
    ];
    $options['paginate_link_class'] = [
      'default' => '',
      'translatable' => FALSE,
    ];
    $options['paginate_link_attributes'] = [
      'default' => '',
      'translatable' => FALSE,
    ];

    // Classes.
    $options['paginate_class'] = [
      'default' => 'alpha-pagination',
      'translatable' => FALSE,
    ];
    $options['paginate_list_class'] = [
      'default' => 'alpha-pagination-list',
      'translatable' => FALSE,
    ];
    $options['paginate_active_class'] = [
      'default' => 'active',
      'translatable' => FALSE,
    ];
    $options['paginate_inactive_class'] = [
      'default' => 'inactive',
      'translatable' => FALSE,
    ];

    // All.
    $options['paginate_all_display'] = [
      'default' => 1,
      'translatable' => FALSE,
    ];
    $options['paginate_all_class'] = [
      'default' => 'all',
      'translatable' => FALSE,
    ];
    $options['paginate_all_label'] = [
      'default' => t('All'),
      'translatable' => TRUE,
    ];
    $options['paginate_all_value'] = [
      'default' => 'all',
      'translatable' => FALSE,
    ];
    $options['paginate_all_position'] = [
      'default' => 'after',
      'translatable' => FALSE,
    ];
    $options['paginate_toggle_empty'] = [
      'default' => 1,
      'translatable' => FALSE,
    ];

    // Numeric.
    $options['paginate_view_numbers'] = [
      'default' => 0,
      'translatable' => FALSE,
    ];
    $options['paginate_numeric_class'] = [
      'default' => 'numeric',
      'translatable' => FALSE,
    ];
    $options['paginate_numeric_divider'] = [
      'default' => 1,
      'translatable' => FALSE,
    ];
    $options['paginate_numeric_divider_class'] = [
      'default' => 'numeric-divider',
      'translatable' => FALSE,
    ];
    $options['paginate_numeric_hide_empty'] = [
      'default' => 1,
      'translatable' => FALSE,
    ];
    $options['paginate_numeric_label'] = [
      'default' => '#',
      'translatable' => FALSE,
    ];
    $options['paginate_numeric_position'] = [
      'default' => 'before',
      'translatable' => FALSE,
    ];
    $options['paginate_numeric_value'] = [
      'default' => implode('+', $this->alphaPagination->getNumbers()),
      'translatable' => FALSE,
    ];

    return $options;
  }

  /**
   * {@inheritdoc}
   */
  public function submitOptionsForm(&$form, FormStateInterface $form_state) {
    // Need to clear cache when options have changed.
    $this->cacheBackend->invalidate($this->alphaPagination->getCid());
    $options = $form_state->getValue('options');

    // Filter attributes for any XSS vulnerabilities before saving.
    if (!empty($options['paginate_link_attributes'])) {
      $form_state->setValue(['options', 'paginate_link_attributes'], Xss::filterAdmin($options['paginate_link_attributes']));
    }
  }

  /**
   * {@inheritdoc}
   */
  public function buildOptionsForm(&$form, FormStateInterface $form_state) {
    parent::buildOptionsForm($form, $form_state);

    // Hide unnecessary label.
    $form['label']['#access'] = FALSE;

    // Global options.
    $base_table = $this->view->storage->get('base_table');
    if ($base_table == 'taxonomy_term_data' || $base_table == 'media_field_data') {
      // Get an array list of all non-image, non-entity or other assorted
      // reference fields.
      $fields = ['name' => 'name'];
    }
    else {
      // Get an array list of all non-image, non-entity or other assorted
      // reference fields.
      $fields = ['title' => 'title'];
    }

    $compound_field_types = ['name'];
    $single_field_types = ['text', 'text_long', 'text_with_summary', 'string', 'string_long'];
    $all_field_types = array_merge($single_field_types, $compound_field_types);
    $all_fields = $this->fieldManager->getFieldMap();
    $baseEntityType = $this->view->getBaseEntityType()->id();
    foreach ($all_fields[$baseEntityType] as $field_name => $field_definition) {
      if (in_array($field_definition['type'], $all_field_types)) {
        if (in_array($field_definition['type'], $compound_field_types)) {
          $field_info = field_info_field($field_name);
          foreach (array_keys($field_info['columns']) as $compoundFieldKey) {
            $compound_field_field_name = sprintf('%s:%s', $field_name, $compoundFieldKey);
            $fields[$baseEntityType . '__' . $compound_field_field_name] = $compound_field_field_name;
          }
        }
        else {
          $fields[$baseEntityType . '__' . $field_name] = $field_name;
        }
      }
    }

    $relationship_options = [];
    foreach ($this->view->display_handler->getHandlers('relationship') as $id => $handler) {
      // Ignore invalid/broken relationships.
      if (empty($handler)) {
        continue;
      }
      $relationship_options[$id] = $handler->adminLabel();
    }

    if (count($relationship_options) > 0) {
      $relationship_options = array_merge(['none' => $this->t('Do not use a relationship')], $relationship_options);
      $form['paginate_view_relationship'] = [
        '#title' => t('Relationship'),
        '#type' => 'select',
        '#options' => $relationship_options,
        '#default_value' => $this->alphaPagination->getOption('paginate_view_relationship'),
      ];
    }

    $form['paginate_view_field'] = [
      '#title' => t('View field to paginate against'),
      '#type' => 'select',
      '#options' => $fields,
      '#default_value' => $this->alphaPagination->getOption('paginate_view_field'),
      '#description' => t('This will be the content field that drives the pagination.'),
    ];

    $form['paginate_toggle_empty'] = [
      '#type' => 'checkbox',
      '#title' => t('Show options without results'),
      '#default_value' => $this->alphaPagination->getOption('paginate_toggle_empty'),
      '#description' => t('Show or hide letters without results'),
    ];

    // Link.
    $form['paginate_link'] = [
      '#type' => 'details',
      '#title' => t('Link'),
      '#collapsible' => TRUE,
    ];

    $form['paginate_link_path'] = [
      '#title' => t('Path'),
      '#type' => 'textfield',
      '#size' => 60,
      '#default_value' => $this->alphaPagination->getOption('paginate_link_path'),
      '#description' => t('This is the path the link will be rendered with. No beginning or ending slashes.'),
      '#fieldset' => 'paginate_link',
    ];

    $form['paginate_link_external'] = [
      '#type' => 'checkbox',
      '#title' => t('External'),
      '#default_value' => $this->alphaPagination->getOption('paginate_link_external'),
      '#description' => t('Indicates whether this is an external link (not processed). If the above path starts with a hash symbol (#), then this option will automatically enable so it can render as a relative link to an anchor on the current page.'),
      '#fieldset' => 'paginate_link',
    ];

    $form['paginate_link_class'] = [
      '#title' => t('Classes'),
      '#type' => 'textfield',
      '#default_value' => $this->alphaPagination->getOption('paginate_link_class'),
      '#description' => t('CSS classes for the link, separated by spaces.'),
      '#fieldset' => 'paginate_link',
    ];

    $form['paginate_link_attributes'] = [
      '#type' => 'textfield',
      '#title' => t('Attributes'),
      '#description' => 'E.g. id|custom-id,role|navigation,data-key|value',
      '#default_value' => $this->alphaPagination->getOption('paginate_link_attributes'),
      '#fieldset' => 'paginate_link',
    ];

    $form['paginate_link_tokens'] = $this->alphaPagination->buildTokenTree('paginate_link');

    // Class options.
    $form['paginate_classes'] = [
      '#type' => 'details',
      '#title' => t('Classes'),
      '#description' => t('Provide additional CSS classes on elements in the pagination; separated by spaces.'),
      '#collapsible' => TRUE,
    ];
    $form['paginate_class'] = [
      '#title' => t('Wrapper'),
      '#type' => 'textfield',
      '#default_value' => $this->alphaPagination->getOption('paginate_class'),
      '#description' => t('The wrapper around the item list.'),
      '#fieldset' => 'paginate_classes',
    ];
    $form['paginate_list_class'] = [
      '#title' => t('Item List'),
      '#type' => 'textfield',
      '#default_value' => $this->alphaPagination->getOption('paginate_list_class'),
      '#description' => t('The item list.'),
      '#fieldset' => 'paginate_classes',
    ];
    $form['paginate_active_class'] = [
      '#title' => t('Active item'),
      '#type' => 'textfield',
      '#default_value' => $this->alphaPagination->getOption('paginate_active_class'),
      '#description' => t('The active list item.'),
      '#fieldset' => 'paginate_classes',
    ];
    $form['paginate_inactive_class'] = [
      '#title' => t('Inactive item'),
      '#type' => 'textfield',
      '#default_value' => $this->alphaPagination->getOption('paginate_inactive_class'),
      '#description' => t('The inactive list item(s) that are not links, a.k.a. "no results".'),
      '#fieldset' => 'paginate_classes',
    ];

    // "All" options.
    $form['paginate_all_options'] = [
      '#type' => 'details',
      '#title' => t('"All" item'),
      '#collapsible' => TRUE,
    ];
    $form['paginate_all_display'] = [
      '#type' => 'select',
      '#title' => t('Display the "All" item'),
      '#options' => [
        0 => t('No'),
        1 => t('Yes'),
      ],
      '#default_value' => $this->alphaPagination->getOption('paginate_all_display'),
      '#description' => t('Displays the "All" link in the pagination.'),
      '#fieldset' => 'paginate_all_options',
    ];
    $form['paginate_all_position'] = [
      '#type' => 'select',
      '#title' => t('Position'),
      '#options' => [
        'before' => t('Before'),
        'after' => t('After'),
      ],
      '#default_value' => $this->alphaPagination->getOption('paginate_all_position'),
      '#description' => t('Determines where the "All" item will show up in the pagination.'),
      '#fieldset' => 'paginate_all_options',
      '#states' => [
        'visible' => [
          [':input[name="options[paginate_all_display]"]' => ['value' => 1]],
        ],
      ],
    ];
    $form['paginate_all_label'] = [
      '#type' => 'textfield',
      '#title' => t('Label'),
      '#default_value' => $this->alphaPagination->getOption('paginate_all_label'),
      '#description' => t('The label to use for display the "All" item in the pagination.'),
      '#fieldset' => 'paginate_all_options',
      '#states' => [
        'visible' => [
          [':input[name="options[paginate_all_display]"]' => ['value' => 1]],
        ],
      ],
    ];
    $form['paginate_all_value'] = [
      '#type' => 'textfield',
      '#title' => t('Value'),
      '#default_value' => $this->alphaPagination->getOption('paginate_all_value'),
      '#description' => t('The value to use to represent all items.'),
      '#fieldset' => 'paginate_all_options',
      '#states' => [
        'visible' => [
          [':input[name="options[paginate_all_display]"]' => ['value' => 1]],
        ],
      ],
    ];
    $form['paginate_all_class'] = [
      '#title' => t('Classes'),
      '#type' => 'textfield',
      '#default_value' => $this->alphaPagination->getOption('paginate_all_class'),
      '#description' => t('CSS classes for "All" item (on <code>&lt;li&gt;</code> element); separated by spaces.'),
      '#fieldset' => 'paginate_all_options',
      '#states' => [
        'visible' => [
          [':input[name="options[paginate_all_display]"]' => ['value' => 1]],
        ],
      ],
    ];

    // "Numeric" options.
    $form['paginate_numeric_options'] = [
      '#type' => 'details',
      '#title' => t('Numeric items'),
      '#collapsible' => TRUE,
    ];

    $form['paginate_view_numbers'] = [
      '#title' => t('Display numeric items'),
      '#type' => 'select',
      '#options' => [
        0 => t('No'),
        1 => t('Individual numbers (0-9)'),
        2 => t('Single label (#)'),
      ],
      '#default_value' => $this->alphaPagination->getOption('paginate_view_numbers'),
      '#description' => t('Displays numeric item(s) in the pagination.'),
      '#fieldset' => 'paginate_numeric_options',
    ];

    // Global numeric options.
    $form['paginate_numeric_class'] = [
      '#title' => t('Classes'),
      '#type' => 'textfield',
      '#default_value' => $this->alphaPagination->getOption('paginate_numeric_class'),
      '#description' => t('CSS classes for numeric item (on <code>&lt;li&gt;</code> element); separated by spaces.'),
      '#fieldset' => 'paginate_numeric_options',
      '#states' => [
        'visible' => [
          [':input[name="options[paginate_view_numbers]"]' => ['value' => 1]],
          'or',
          [':input[name="options[paginate_view_numbers]"]' => ['value' => 2]],
        ],
      ],
    ];

    $form['paginate_numeric_position'] = [
      '#type' => 'select',
      '#title' => t('Position'),
      '#options' => [
        'before' => t('Before'),
        'after' => t('After'),
      ],
      '#default_value' => $this->alphaPagination->getOption('paginate_numeric_position'),
      '#description' => t('Determines whether numeric items are shown before or after alphabetical links in the pagination.'),
      '#fieldset' => 'paginate_numeric_options',
      '#states' => [
        'visible' => [
          [':input[name="options[paginate_view_numbers]"]' => ['value' => 1]],
          'or',
          [':input[name="options[paginate_view_numbers]"]' => ['value' => 2]],
        ],
      ],
    ];

    $form['paginate_numeric_hide_empty'] = [
      '#title' => t('Hide all numeric item(s) if empty'),
      '#description' => t('Will not render any numeric item(s) if there are no results that start with numeric values.'),
      '#type' => 'checkbox',
      '#default_value' => $this->alphaPagination->getOption('paginate_numeric_hide_empty'),
      '#fieldset' => 'paginate_numeric_options',
      '#states' => [
        'visible' => [
          [':input[name="options[paginate_view_numbers]"]' => ['value' => 1]],
          'or',
          [':input[name="options[paginate_view_numbers]"]' => ['value' => 2]],
        ],
      ],
    ];

    // Individual numeric items.
    $form['paginate_numeric_divider'] = [
      '#type' => 'checkbox',
      '#title' => t('Show divider'),
      '#default_value' => $this->alphaPagination->getOption('paginate_numeric_divider'),
      '#description' => t('Will render a specific divider item before or after the numeric items have been render, based on position.'),
      '#fieldset' => 'paginate_numeric_options',
      '#states' => [
        'visible' => [
          ':input[name="options[paginate_view_numbers]"]' => ['value' => 1],
        ],
      ],
    ];

    $form['paginate_numeric_divider_class'] = [
      '#title' => t('Divider class'),
      '#type' => 'textfield',
      '#default_value' => $this->alphaPagination->getOption('paginate_numeric_divider_class'),
      '#description' => t('The class to use for the numeric divider list item.'),
      '#fieldset' => 'paginate_numeric_options',
      '#states' => [
        'visible' => [
          ':input[name="options[paginate_view_numbers]"]' => ['value' => 1],
          ':input[name="options[paginate_numeric_divider]"]' => ['checked' => TRUE],
        ],
      ],
    ];

    // Single numeric item.
    $form['paginate_numeric_label'] = [
      '#title' => t('Label'),
      '#type' => 'textfield',
      '#default_value' => $this->alphaPagination->getOption('paginate_numeric_label'),
      '#description' => t('The label to use to represent all numeric values.'),
      '#fieldset' => 'paginate_numeric_options',
      '#states' => [
        'visible' => [
          ':input[name="options[paginate_view_numbers]"]' => ['value' => 2],
        ],
      ],
    ];

    $form['paginate_numeric_value'] = [
      '#title' => t('Value'),
      '#type' => 'textfield',
      '#default_value' => $this->alphaPagination->getOption('paginate_numeric_value'),
      '#description' => t('The value to use to represent all numeric values (i.e. URL value).'),
      '#fieldset' => 'paginate_numeric_options',
      '#states' => [
        'visible' => [
          ':input[name="options[paginate_view_numbers]"]' => ['value' => 2],
        ],
      ],
    ];

  }

  /**
   * {@inheritdoc}
   */
  public function postExecute(&$values) {
    $this->alphaPagination->ensureQuery();
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
  public function render($empty = FALSE) {
    // Create the wrapper.
    $wrapper = [
      '#theme_wrappers' => ['container__alpha_pagination__wrapper'],
      '#attributes' => [],
      '#attached' => [
        'library' => [
          'alpha_pagination/alpha_pagination',
        ],
      ],
    ];

    $this->alphaPagination->addClasses($this->alphaPagination->getOption('paginate_class'), $wrapper['#attributes']);

    // Iterate over the alphabet and populate the items for the item list.
    $items = [];
    foreach ($this->alphaPagination->getCharacters() as $character) {
      // Add special numeric divider.
      if ($character->getValue() === '-' && $this->alphaPagination->getOption('paginate_view_numbers') !== '2' && $this->alphaPagination->getOption('paginate_numeric_divider')) {
        // Add an empty list item.
        $item = ['data' => ''];
        $this->alphaPagination->addClasses($this->alphaPagination->getOption('paginate_numeric_divider_class'), $item);
        $items[] = $item;
      }
      elseif ($item = $character->build()) {
        $item['#wrapper_attributes'] = [];

        // Add the necessary classes for item.
        if ($character->isAll()) {
          $this->alphaPagination->addClasses($this->alphaPagination->getOption('paginate_all_class'), $item['#wrapper_attributes']);
        }

        if ($character->isNumeric()) {
          $this->alphaPagination->addClasses($this->alphaPagination->getOption('paginate_numeric_class'), $item['#wrapper_attributes']);
        }

        if ($character->isActive()) {
          $this->alphaPagination->addClasses($this->alphaPagination->getOption('paginate_active_class'), $item['#wrapper_attributes']);
        }
        elseif (!$character->isEnabled()) {
          $this->alphaPagination->addClasses($this->alphaPagination->getOption('paginate_inactive_class'), $item['#wrapper_attributes']);
        }

        // Add the constructed item to the list.
        $items[] = $item;
      }
    }

    // Sanitize any classes provided for the item list.
    $item_list = [
      '#theme' => 'item_list__alpha_pagination',
      '#attributes' => [],
      '#items' => $items,
    ];
    $this->alphaPagination->addClasses($this->alphaPagination->getOption('paginate_list_class'), $item_list['#attributes']);

    // Append the item list to the wrapper.
    $wrapper[] = $item_list;

    return $wrapper;
  }

  /**
   * {@inheritdoc}
   */
  public function validate() {
    return $this->alphaPagination->validate();
  }

}
