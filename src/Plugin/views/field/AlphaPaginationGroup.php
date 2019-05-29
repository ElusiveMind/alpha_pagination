<?php

namespace Drupal\alpha_pagination\Plugin\views\field;

use Drupal\views\Plugin\views\field\FieldPluginBase;
use Drupal\views\ResultRow;
use Drupal\alpha_pagination\AlphaPagination;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Render\Element;
use Drupal\Component\Utility\Unicode;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\Core\Utility\Token;

/**
 * A handler to provide a field that generates alpha pagination values.
 *
 * @ingroup views_field_handlers
 *
 * @ViewsField("alpha_pagination_group")
 */
class AlphaPaginationGroup extends FieldPluginBase {

  /**
   * @var \AlphaPagination
   */
  protected $alphaPagination;

/**
   * Constructs a new RenderedEntity object.
   *
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param array $plugin_definition
   *   The plugin implementation definition.
   * @param \alpha_pagination\AlphaPagination $alphaPagination
   *    The AlphaPagination service.
   * @param \Drupal\Core\Utility\Token $token
   *   The token service.
   */
  public function __construct(array $configuration, $plugin_id, array $plugin_definition, AlphaPagination $alphaPagination, Token $token) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->alphaPagination = $alphaPagination;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static($configuration, $plugin_id, $plugin_definition, $container
      ->get('alpha_pagination'), $container
      ->get('token'));
  }

  /**
   * {@inheritdoc}
   */
  function defineOptions() {
    $options = parent::defineOptions();
    $options['label'] = '';
    $options['element_label_colon'] = FALSE;
    $options['exclude']['default'] = TRUE;
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  function submitOptionsForm(&$form, FormStateInterface $form_state) {
    parent::submitOptionsForm($form, $form_state);

    // Since this is an automated field, hide all form elements.
    foreach (Element::children($form) as $child) {
      // Allow the custom admin label since that's purely UI related.
      if ($child === 'more' || $child === 'ui_name') {
        continue;
      }
      $form[$child]['#access'] = FALSE;
    }
  }

  /**
   * {@inheritdoc}
   */
  function query() {
    // Intentionally left empty because this doesn't actually alter the query.
  }

  /**
   * {@inheritdoc}
   */
  function render(ResultRow $values) {
    $areas = $this->alphaPagination->getAreaHandlers();

    // Immediately return if there is no handler.
    if (!$areas) {
      return '';
    }

    $this->alphaPagination->setHandler($areas[0]);
    $path = $this->alphaPagination->getOption('paginate_link_path');
    list($entityType, $field_name) = explode('__',$this->alphaPagination->getOption('paginate_view_field'), 2);
    if (!isset($this->view->field[$field_name])) {
      return '';
    }

    /** @var Drupal\views\Plugin\views\field\FieldPluginBase */
    $field = $this->view->field[$field_name];

    // Render field if it hasn't already.
    if (empty($field->last_render)) {
      $field->renderText($values);
    }

    // Return just the first character of the value.
    $value = $this->alphaPagination->getValue(!empty($field->last_render) ? Unicode::ucfirst(substr(strip_tags($field->last_render), 0, 1)) : '');
    $label = $this->alphaPagination->getLabel($value);

    // Prepend an anchor if the link path starts with #. Using the "link"
    // element will cause an empty href attribute to be printed. This can cause
    // issues with certain JavaScript or CSS styling that targets if that
    // attribute is simply present, regardless if it's empty. To avoid that,
    // use the "html_tag" type so only the name attribute is printed.
    if ($value && $path && $path[0] === '#') {
      $name = $this->token->replace(substr($path, 1), $alpha_pagination->getTokens($value));
      $label = [
        '#type' => 'html_tag',
        '#theme' => 'html_tag__alpha_pagination__anchor',
        '#tag' => 'a',
        '#value' => '',
        '#attributes' => ['name' => $name],
        '#suffix' => $label,
      ];
    }

    return $label;
  }

  /**
   * {@inheritdoc}
   */
  public function validate() {
    return $this->alphaPagination->validate();
  }

}
