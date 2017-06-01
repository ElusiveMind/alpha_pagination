<?php

/**
 * A handler to provide a field that generates alpha pagination values.
 *
 * @ingroup views_field_handlers
 */
class views_handler_field_alpha_pagination_group extends \views_handler_field {

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
  function option_definition() {
    $options = parent::option_definition();
    $options['label'] = '';
    $options['element_label_colon'] = FALSE;
    $options['exclude']['default'] = TRUE;
    return $options;
  }

  /**
   * {@inheritdoc}
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    // Since this is an automated field, hide all form elements.
    foreach (element_children($form) as $child) {
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
  function render($values) {
    $areas = $this->helper->getAreaHandlers();

    // Immediately return if there is no handler.
    if (!$areas) {
      return '';
    }

    $helper = new AlphaPaginationHelper($areas[0]);
    $path = $helper->getOption('paginate_link_path');
    $field_name = $helper->getOption('paginate_view_field');

    if (!isset($this->view->field[$field_name])) {
      return '';
    }

    /** @var \views_handler_field $field */
    $field = $this->view->field[$field_name];

    // Render field if it hasn't already.
    if (empty($field->last_render_text)) {
      $field->render($values);
    }

    // Return just the first character of the value.
    $value = !empty($field->last_render_text) ? drupal_ucfirst(substr(strip_tags($field->last_render_text), 0, 1)) : '';

    // Prepend an anchor if the link path starts with #. Using the "link"
    // element will cause an empty href attribute to be printed. This can cause
    // issues with certain JavaScript or CSS styling that targets if that
    // attribute is simply present, regardless if it's empty. To avoid that,
    // use the "html_tag" type so only the name attribute is printed.
    if ($value && $path && $path[0] === '#') {
      $name = token_replace(substr($path, 1), $helper->getTokens($value));
      $value = [
        '#type' => 'html_tag',
        '#theme' => 'html_tag__alpha_pagination__anchor',
        '#tag' => 'a',
        '#value' => '',
        '#attributes' => ['name' => $name],
        '#suffix' => $value,
      ];
    }

    return $value;
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
  public function validate() {
    return $this->helper->validate();
  }

}
