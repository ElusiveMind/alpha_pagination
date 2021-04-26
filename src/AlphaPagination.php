<?php

namespace Drupal\alpha_pagination;

use Drupal\alpha_pagination\Plugin\views\area\AlphaPaginationArea;
use Drupal\Component\Utility\Html;
use Drupal\Component\Transliteration\TransliterationInterface;
use Drupal\Component\Utility\Crypt;
use Drupal\Core\Cache\CacheBackendInterface;
use Drupal\Core\Database\Connection;
use Drupal\Core\Entity\EntityFieldManagerInterface;
use Drupal\Core\Extension\ModuleHandlerInterface;
use Drupal\Core\Language\LanguageManagerInterface;
use Drupal\Core\Utility\Token;
use Drupal\views\Views;
use Drupal\views\Plugin\views\ViewsHandlerInterface;

/**
 * A base views handler for alpha pagination.
 */
class AlphaPagination {

  /**
   * The AlphaPagination Views Handler.
   *
   * @var \Drupal\views\Plugin\views\ViewsHandlerInterface
   */
  protected $handler;

  /**
   * The Entity Field Manager Interface.
   *
   * @var \Drupal\Core\Entity\EntityFieldManagerInterface
   */
  protected $fieldManager;

  /**
   * The Language ID.
   *
   * @var string
   */
  protected $language;

  /**
   * The ModuleHandler Interface.
   *
   * @var \Drupal\Core\Extension\ModuleHandlerInterface
   */
  protected $moduleHandler;

  /**
   * The cache backend service.
   *
   * @var \Drupal\Core\Cache\CacheBackendInterface
   */
  protected $cacheBackend;

  /**
   * The Transliteration Interface.
   *
   * @var \Drupal\Component\Transliteration\TransliterationInterface
   */
  protected $transliteration;

  /**
   * The database connection.
   *
   * @var \Drupal\Core\Database\Connection
   */
  protected $database;

  /**
   * The token replacement instance.
   *
   * @var \Drupal\Core\Utility\Token
   */
  protected $token;

  /**
   * The url used for links.
   *
   * @var \Drupal\Core\Url
   */
  protected $url;

  /**
   * Contructs the AlphaPagination object.
   *
   * @param \Drupal\Core\Entity\EntityFieldManagerInterface $field_manager
   *   The Entity Field Manager Interface.
   * @param \Drupal\Core\Language\LanguageManagerInterface $language_manager
   *   The language manager service.
   * @param \Drupal\Core\Extension\ModuleHandlerInterface $module_handler
   *   The ModuleHander Interface.
   * @param \Drupal\Core\Cache\CacheBackendInterface $cache_backend
   *   The cache backend service.
   * @param \Drupal\Component\Transliteration\TransliterationInterface $transliteration
   *   A PHPTransliteration object.
   * @param \Drupal\Core\Database\Connection $database
   *   The database connection.
   * @param \Drupal\Core\Utility\Token $token
   *   The token service.
   */
  public function __construct(EntityFieldManagerInterface $field_manager, LanguageManagerInterface $language_manager, ModuleHandlerInterface $module_handler, CacheBackendInterface $cache_backend, TransliterationInterface $transliteration, Connection $database, Token $token) {
    $this->fieldManager = $field_manager;
    $this->language = $language_manager->getCurrentLanguage()->getId();
    $this->moduleHandler = $module_handler;
    $this->cacheBackend = $cache_backend;
    $this->transliteration = $transliteration;
    $this->database = $database;
    $this->token = $token;
  }

  /**
   * Sets the view handler.
   *
   * @param \Drupal\views\Plugin\views\ViewsHandlerInterface $handler
   *   The views handler.
   */
  public function setHandler(ViewsHandlerInterface $handler) {
    $this->handler = $handler;
  }

  /**
   * {@inheritdoc}
   */
  public function __sleep() {
    $this->_handler = implode(':', [
      $this->handler->view->id(),
      $this->handler->view->current_display,
      $this->handler->areaType,
      $this->handler->realField ?: $this->handler->field,
      $this->language,
    ]);

    return ['_handler'];
  }

  /**
   * {@inheritdoc}
   */
  public function __wakeup() {
    list($name, $display_id, $type, $id, $language) = explode(':', $this->_handler);
    $view = Views::getView($name);
    $view->setDisplay($display_id);
    $this->handler = $view->display_handler->getHandler($type, $id);

    $this->language = $language;
    unset($this->_handler);
  }

  /**
   * Add classes to an attributes array from a view option.
   *
   * @param string[]|string $classes
   *   An array of classes or a string of classes.
   * @param array $attributes
   *   An attributes array to add the classes to, passed by reference.
   *
   * @return array
   *   An array of classes to be used in a render array.
   */
  public function addClasses($classes, array &$attributes) {
    $processed = [];

    // Sanitize any classes provided for the item list.
    foreach ((array) $classes as $v) {
      foreach (array_filter(explode(' ', $v)) as $vv) {
        $processed[] = Html::cleanCssIdentifier($vv);
      }
    }

    // Don't add any classes if it's empty, which will add an empty attribute.
    if ($processed) {
      if (!isset($attributes['class'])) {
        $attributes['class'] = [];
      }
      $attributes['class'] = array_unique(array_merge($attributes['class'], $processed));
    }

    return $classes;
  }

  /**
   * Builds a render array for displaying tokens.
   *
   * @param string $fieldset
   *   The #fieldset name to assign.
   *
   * @return array
   *   The render array for the token info.
   */
  public function buildTokenTree($fieldset = NULL) {
    static $build;

    if (!isset($build)) {
      if ($this->moduleHandler->moduleExists('token')) {
        $build = [
          '#type' => 'container',
          '#title' => t('Browse available tokens'),
        ];
        $build['help'] = [
          '#theme' => 'token_tree_link',
          '#token_types' => ['alpha_pagination'],
          '#global_types' => TRUE,
          '#dialog' => TRUE,
        ];
      }
      else {
        $build = [
          '#type' => 'fieldset',
          '#title' => 'Available tokens',
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
        ];
        $items = [];
        $token_info = alpha_pagination_token_info();
        foreach ($token_info['tokens'] as $_type => $_tokens) {
          foreach ($_tokens as $_token => $_data) {
            $items[] = "[$_type:$_token] - {$_data['description']}";
          }
        }
        $build['help'] = [
          '#theme' => 'item_list',
          '#items' => $items,
        ];
      }
    }

    return isset($fieldset) ? ['#fieldset' => $fieldset] + $build : $build;
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
  public function ensureQuery() {
    if (!$this->getOption('query') && !empty($this->handler->view->build_info['query'])) {
      /** @var \SelectQuery $query */
      $query = $this->handler->view->build_info['query'];
      $quoted = $query->getArguments();
      foreach ($quoted as $key => $val) {
        if (is_array($val)) {
          $quoted[$key] = implode(', ', array_map([
            $this->database,
            'quote',
          ], $val));
        }
        else {
          $quoted[$key] = $this->database->quote($val);
        }
      }
      $this->handler->options['query'] = Html::escape(strtr($query, $quoted));
    }
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
  public function getAlphabet($langcode = NULL) {

    // Default (English).
    static $default = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'];
    static $alphabets;

    // If the langcode is not explicitly specified, default to global langcode.
    if (!isset($langcode)) {
      $langcode = $this->language;
    }

    // Retrieve alphabets.
    if (!isset($alphabets)) {
      // Attempt to retrieve from database cache.
      $cid = "alpha_pagination:alphabets";
      if (($cache = $this->cacheBackend->get($cid)) && !empty($cache->data)) {
        $alphabets = $cache->data;
      }
      // Build alphabets.
      else {
        // Arabic.
        $alphabets['ar'] = ['ا', 'ب', 'ت', 'ث', 'ج', 'ح', 'خ', 'د', 'ذ', 'ر', 'ز', 'س', 'ش', 'ص', 'ض', 'ط', 'ظ', 'ع', 'غ', 'ف', 'ق', 'ك', 'ل', 'م', 'ن', 'و', 'ه', 'ي'];

        // English. Initially the default value, but can be modified in alter.
        $alphabets['en'] = $default;

        // Русский (Russian).
        $alphabets['ru'] = ['А', 'Б', 'В', 'Г', 'Д', 'Е', 'Ё', 'Ж', 'З', 'И', 'Й', 'К', 'Л', 'М', 'Н', 'О', 'П', 'Р', 'С', 'Т', 'У', 'Ф', 'Х', 'Ц', 'Ч', 'Ш', 'Щ', 'Ы', 'Э', 'Ю', 'Я'];

        // Allow modules and themes to alter alphabets.
        $this->moduleHandler->alter('alpha_pagination_alphabet', $alphabets, $this);

        // Cache the alphabets.
        $this->cacheBackend->set($cid, $alphabets);
      }
    }

    // Return alphabet based on langcode.
    return isset($alphabets[$langcode]) ? $alphabets[$langcode] : $default;
  }

  /**
   * Retrieves all available alpha pagination areas.
   *
   * @param array $types
   *   The handler types to search.
   *
   * @return \Drupal\alpha_pagination\Plugin\views\area\AlphaPaginationArea[]
   *   An array of alpha pagination areas.
   */
  public function getAreaHandlers(array $types = ['header', 'footer']) {
    $areas = [];
    foreach ($types as $type) {
      foreach ($this->handler->displayHandler->getHandlers($type) as $handler) {
        if ($handler instanceof AlphaPaginationArea) {
          $areas[] = $handler;
        }
      }
    }
    return $areas;
  }

  /**
   * Retrieves the characters used to populate the pagination item list.
   *
   * @return \Drupal\alpha_pagination\AlphaPaginationCharacter[]
   *   An associative array containing AlphaPaginationCharacter objects, keyed
   *   by its value.
   */
  public function getCharacters() {
    /** @var \Drupal\alpha_pagination\AlphaPaginationCharacter[] $characters */
    static $characters;

    if (!isset($characters)) {
      $all = $this->getOption('paginate_all_display') === '1' ? $this->getOption('paginate_all_label', t('All')) : '';
      $all_value = $this->getOption('paginate_all_value', 'all');
      $numeric_label = $this->getOption('paginate_numeric_label');
      $numeric_type = $this->getOption('paginate_view_numbers', '0');
      $numeric_value = $this->getOption('paginate_numeric_value');
      $numeric_divider = $numeric_type !== '2' && $this->getOption('paginate_numeric_divider') ? ['-' => ''] : [];

      // Check to see if this query is cached. If it is, then just pull our
      // results set from it so that things can move quickly through here. We're
      // caching in the event the view has a very large result set.
      $cid = $this->getCid();
      if (($cache = $this->cacheBackend->get($cid)) && !empty($cache->data)) {
        $characters = $cache->data;
      }
      else {
        // Add alphabet characters.
        foreach ($this->getAlphabet() as $value) {
          $characters[$value] = $value;
        }

        // Append or prepend numeric item(s).
        $numeric = [];
        if ($numeric_type !== '0') {

          // Determine type of numeric items.
          if ($numeric_type === '2') {
            $numeric[$numeric_value] = Html::escape($numeric_label);
          }
          else {
            foreach ($this->getNumbers() as $value) {
              $numeric[$value] = $value;
            }
          }

          // Merge in numeric items.
          if ($this->getOption('paginate_numeric_position') === 'after') {
            $characters = array_merge($characters, $numeric_divider, $numeric);
          }
          else {
            $characters = array_merge($numeric, $numeric_divider, $characters);
          }
        }

        // Append or prepend the "all" item.
        if ($all) {
          if ($this->getOption('paginate_all_position') === 'before') {
            $characters = [$all_value => $all] + $characters;
          }
          else {
            $characters[$all_value] = $all;
          }
        }

        // Convert characters to objects.
        foreach ($characters as $value => $label) {
          $characters[$value] = new AlphaPaginationCharacter($this, $label, $value);
        }

        // Determine enabled prefixes.
        $prefixes = $this->getEntityPrefixes();
        foreach ($prefixes as $value) {
          // Ensure numeric prefixes use the numeric label, if necessary.
          if ($this->isNumeric($value) && $numeric_type === '2') {
            $value = $numeric_value;
          }
          if (isset($characters[$value])) {
            $characters[$value]->setEnabled(TRUE);
          }
        }

        // Remove all empty prefixes.
        if (!$this->getOption('paginate_toggle_empty')) {
          // Ensure "all" and numeric divider objects aren't removed.
          if ($all) {
            $prefixes[] = $all_value;
          }
          if ($numeric_divider) {
            $prefixes[] = '-';
          }
          // Determine if numeric results are not empty.
          $characters = array_filter(array_intersect_key($characters, array_flip($prefixes)));
        }

        // Remove all numeric values if they're all empty.
        if ($this->getOption('paginate_numeric_hide_empty')) {
          // Determine if numeric results are not empty.
          $numeric_results = array_filter(array_intersect_key($characters, array_flip($numeric)));
          if (!$numeric_results) {
            $characters = array_diff_key($characters, array_flip($numeric));
          }
        }

        // Cache the results.
        $this->cacheBackend->set($cid, $characters);
      }

      // Default current value to "all", if enabled, or the first character.
      $current = $all ? $all_value : '';

      // Attempt to determine if a valid argument was provided.
      $arg_count = count($this->handler->view->args);
      if ($arg_count) {
        $arg = $this->handler->view->args[$arg_count - 1];
        if ($arg && in_array($arg, array_keys($characters))) {
          $current = $arg;
        }
      }

      // Determine the first active character.
      if ($current) {
        foreach ($characters as $character) {
          if ($character->isNumeric() && $numeric_type === '2' ? $numeric_value === $current : $character->getValue() === $current) {
            $character->setActive(TRUE);
            break;
          }
        }
      }
    }

    return $characters;
  }

  /**
   * Retrieves a cache identifier for the view, display and query, if set.
   *
   * @return string
   *   A cache identifier.
   */
  public function getCid() {
    $this->ensureQuery();
    $data = [
      'langcode' => $this->language,
      'view' => $this->handler->view->id(),
      'display' => $this->handler->view->getDisplay()->getPluginId(),
      'query' => $this->getOption('query') ? md5($this->getOption('query')) : '',
      'options' => $this->handler->options,
    ];
    return 'alpha_pagination:' . Crypt::hashBase64(serialize($data));
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
  public function getEntityIds() {
    $this->ensureQuery();
    $query_parts = explode("\n", $this->getOption('query'));

    // Get the base field. This will change depending on the type of view we
    // are putting the paginator on and eventually if we are using a
    // relationship to access it.
    $relationship = $this->getOption('paginate_view_relationship');
    if ($relationship == 'none') {
      $base_field = $this->handler->view->storage->get('base_field');
    }
    else {
      // Inspired from core/modules/views_ui/src/Form/Ajax/ConfigHandler.php
      // buildForm further exploration needed when chaining relationships.
      $relationship_handler = $this->handler->view->getHandler($this->handler->view->current_display, 'relationship', $relationship);
      $data = Views::viewsData()->get($relationship_handler['table']);
      if (
        isset($data[$relationship_handler['field']]['relationship']['base']) &&
        $this->handler->view->storage->get('base_table') == $data[$relationship_handler['field']]['relationship']['base']
      ) {
        $base_field = implode('_', [
          $data[$relationship_handler['field']]['relationship']['base'],
          $relationship_handler['table'],
          $data[$relationship_handler['field']]['relationship']['base field'],
        ]);
      }
    }
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
    $query = implode("\n", $query_parts);
    $query = str_replace("&#039;", '\'', $query);
    $query = str_replace("&amp;", '&', $query);
    $query = str_replace("&lt;", '<', $query);
    $query = str_replace("&gt;", '>', $query);
    $query = str_replace("&quot;", '', $query);

    // Based on our query, get the list of entity identifiers that are affected.
    // These will be used to generate the pagination items.
    $entity_ids = [];
    $result = $this->database->query($query);
    while ($data = $result->fetchObject()) {
      $entity_ids[] = $data->$base_field;
    }
    return $entity_ids;
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
  public function getEntityPrefixes() {
    $prefixes = [];

    if ($entity_ids = $this->getEntityIds()) {
      switch ($this->getOption('paginate_view_field')) {
        case 'name':
          $table = $this->handler->view->storage->get('base_table');
          $where = $this->handler->view->storage->get('base_field');

          // Extract the "name" field from the entity property info.
          $entity_type = $this->handler->view->getBaseEntityType->id();
          $entity_info = $this->fieldManager->getBaseFieldDefinitions($entity_type);
          $field = isset($entity_info['properties']['name']['schema field']) ? $entity_info['properties']['name']['schema field'] : 'name';
          break;

        case 'title':
          $table = $this->handler->view->storage->get('base_table');
          $where = $this->handler->view->storage->get('base_field');

          // Extract the "title" field from the entity property info.
          $entity_type = $this->handler->view->getBaseEntityType()->id();
          $entity_info = $this->fieldManager->getBaseFieldDefinitions($entity_type);
          $field = isset($entity_info['properties']['title']['schema field']) ? $entity_info['properties']['title']['schema field'] : 'title';
          break;

        default:
          if (strpos($this->getOption('paginate_view_field'), ':') === FALSE) {
            // Format field name and table for single value fields.
            list($entityType, $field_name) = explode('__', $this->getOption('paginate_view_field'), 2);
            $field = $field_name . '_value';
            $table = $this->getOption('paginate_view_field');
          }
          else {
            // Format field name and table for compound value fields.
            list($entityType, $field_name) = explode('__', $this->getOption('paginate_view_field'), 2);
            $field = str_replace(':', '_', $field_name);
            $field_name_components = explode(':', $field_name);
            $table = $field_name_components[0];
          }
          $where = 'entity_id';
          break;
      }
      $result = $this->database->query('SELECT DISTINCT(SUBSTR(' . $field . ', 1, 1)) AS prefix
                          FROM {' . $table . '}
                          WHERE ' . $where . ' IN ( :nids[] )', [':nids[]' => $entity_ids]);
      while ($data = $result->fetchObject()) {
        $prefixes[] = is_numeric($data->prefix) ? $data->prefix : mb_strtoupper($this->transliteration->transliterate($data->prefix));
      }
    }
    return array_unique(array_filter($prefixes));
  }

  /**
   * Retrieves the proper label for a character.
   *
   * @param mixed $value
   *   The value of the label to retrieve.
   *
   * @return string
   *   The label.
   */
  public function getLabel($value) {
    $characters = $this->getCharacters();

    // Return an appropriate numeric label.
    if ($this->getOption('paginate_view_numbers') === '2' && $this->isNumeric($value)) {
      return $characters[$this->getOption('paginate_numeric_value')]->getLabel();
    }
    elseif (isset($characters[$value])) {
      return $characters[$value]->getLabel();
    }

    // Return the original value.
    return $value;
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
  public function getNumbers($langcode = NULL) {
    // Default (English).
    static $default = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9'];
    static $numbers;

    // If the langcode is not explicitly specified, default to global langcode.
    if (!isset($langcode)) {
      $langcode = $this->language;
    }

    // Retrieve numbers.
    if (!isset($numbers)) {
      // Attempt to retrieve from database cache.
      $cid = "alpha_pagination:numbers";
      if (($cache = $this->cacheBackend->get($cid)) && !empty($cache->data)) {
        $numbers = $cache->data;
      }
      // Build numbers.
      else {
        // English. Initially the default value, but can be modified in alter.
        $numbers['en'] = $default;

        // Allow modules and themes to alter numbers.
        $this->moduleHandler->alter('alpha_pagination_numbers', $numbers, $this);

        // Cache the numbers.
        $this->cacheBackend->set($cid, $numbers);
      }
    }

    // Return numbers based on langcode.
    return isset($numbers[$langcode]) ? $numbers[$langcode] : $default;
  }

  /**
   * Retrieves an option from the view handler.
   *
   * @param string $name
   *   The option name to retrieve.
   * @param mixed $default
   *   The default value to return if not set.
   *
   * @return string
   *   The option value or $default if not set.
   */
  public function getOption($name, $default = '') {
    return (string) (isset($this->handler->options[$name]) ? $this->handler->options[$name] : $default);
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
  public function getTokens($value = NULL) {
    return [
      'alpha_pagination' => [
        'path' => $this->getUrl(),
        'value' => $value,
      ],
    ];
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
  public function getUrl() {
    if (empty($this->url)) {
      if (!empty($this->handler->view->override_url)) {
        $this->url = $this->handler->view->override_url;
        return $this->url;
      }

      $path = $this->handler->view->getPath();
      $args = $this->handler->view->args;
      // Exclude arguments that were computed, not passed on the URL.
      $position = 0;
      if (!empty($this->handler->view->argument)) {
        foreach ($this->handler->view->argument as $argument_id => $argument) {
          if (!empty($argument->options['default_argument_skip_url'])) {
            unset($args[$position]);
          }
          $position++;
        }
      }

      // Don't bother working if there's nothing to do:
      if (empty($path) || (empty($args) && strpos($path, '%') === FALSE)) {
        $path = \Drupal::service('path.current')->getPath();
        $pieces = explode('/', $path);
        // If getPath returns heading slash, remove it as it will ba
        // added later.
        if ($pieces[0] == '') {
          array_shift($pieces);
        }
        if (array_key_exists(end($pieces), $this->getCharacters())) {
          array_pop($pieces);
        }
        $this->url = implode('/', $pieces);
        return $this->url;
      }

      $pieces = [];
      $argument_keys = isset($this->handler->view->argument) ? array_keys($this->handler->view->argument) : [];
      $id = current($argument_keys);
      foreach (explode('/', $path) as $piece) {
        if ($piece != '%') {
          $pieces[] = $piece;
        }
        else {
          if (empty($args)) {
            // Try to never put % in a url; use the wildcard instead.
            if ($id && !empty($this->handler->view->argument[$id]->options['exception']['value'])) {
              $pieces[] = $this->handler->view->argument[$id]->options['exception']['value'];
            }
            else {
              // Gotta put something if there just isn't one.
              $pieces[] = '*';
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
      $this->url = implode('/', $pieces);
    }
    return $this->url;
  }

  /**
   * Retrieves the proper value for a character.
   *
   * @param mixed $value
   *   The value to retrieve.
   *
   * @return string
   *   The value.
   */
  public function getValue($value) {
    $characters = $this->getCharacters();

    // Return an appropriate numeric label.
    if ($this->getOption('paginate_view_numbers') === '2' && $this->isNumeric($value)) {
      return $characters[$this->getOption('paginate_numeric_value')]->getValue();
    }
    elseif (isset($characters[$value])) {
      return $characters[$value]->getLabel();
    }

    // Return the original value.
    return $value;
  }

  /**
   * Determines if value is "numeric".
   *
   * @param string $value
   *   The value to test.
   *
   * @return bool
   *   TRUE or FALSE
   */
  public function isNumeric($value) {
    return ($this->getOption('paginate_view_numbers') === '2' && $value === $this->getOption('paginate_numeric_value')) || in_array($value, $this->getNumbers());
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
  public function parseAttributes($string = NULL, array $tokens = []) {
    $attributes = [];
    if (!empty($string)) {
      $parts = explode(',', $string);
      foreach ($parts as $attribute) {
        if (strpos($attribute, '|') !== FALSE) {
          list($key, $value) = explode('|', $this->token->replace($attribute, $tokens, ['clear' => TRUE]));
          $attributes[$key] = $value;
        }
      }
    }
    return $attributes;
  }

  /**
   * {@inheritdoc}
   */
  public function validate() {
    $name = $this->handler->view->id();
    $display_id = $this->handler->view->current_display;

    // Immediately return if display doesn't have the handler in question.
    $items = $this->handler->view->getHandlers($this->handler->areaType, $display_id);
    $field = $this->handler->realField ?: $this->handler->field;
    if (!isset($items[$field])) {
      return [];
    }

    static $errors = [];
    if (!isset($errors["$name:$display_id"])) {
      $errors["$name:$display_id"] = [];

      // Show an error if not found.
      $areas = $this->getAreaHandlers();
      if (!$areas) {
        $errors["$name:$display_id"][] = t('The view "@name:@display" must have at least one configured alpha pagination area in either the header or footer to use "@field".', [
          '@field' => $field,
          '@name' => $this->handler->view->id(),
          '@display' => $this->handler->view->current_display,
        ]);
      }
      // Show an error if there is more than one instance.
      elseif (count($areas) > 1) {
        $errors["$name:$display_id"][] = t('The view "@name:@display" can only have one configured alpha pagination area in either the header or footer.', [
          '@name' => $this->handler->view->id(),
          '@display' => $this->handler->view->current_display,
        ]);
      }

    }

    return $errors["$name:$display_id"];
  }

}
