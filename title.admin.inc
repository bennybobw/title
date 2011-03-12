<?php

/**
 * @file
 * Admin page callbacks for the Title module.
 */

/**
 * Provide settings to enable title field.
 */
function title_form_field_ui_overview(&$form, &$form_state) {
  $entity_info = entity_get_info($form['#entity_type']);

  if (!empty($entity_info['field replacement'])) {
    $field_replacement_info = $entity_info['field replacement'];
    $admin_path = _field_ui_bundle_admin_path($form['#entity_type'], $form['#bundle']);
    $form['fields']['#header'][6]['colspan'] += 1;

    foreach (element_children($form['fields']) as $field_name) {
      if (isset($field_replacement_info[$field_name])) {
        $form['fields'][$field_name]['field_replacement'] = array(
          '#type' => 'link',
          '#title' => t('replace'),
          '#href' => $admin_path . '/fields/replace/' . $field_name,
          '#options' => array('attributes' => array('title' => t('Replace %field with a field instance.', array('%field' => $field_name)))),
        );
      }
      else {
        $form['fields'][$field_name]['field_replacement'] = array();
      }
    }
  }
}

/**
 * Generate a field replacement form.
 */
function title_field_replacement_form($form, $form_state, $entity_type, $bundle, $field_name) {
  $bundle_name = field_extract_bundle($entity_type, $bundle);
  $entity_info = entity_get_info($entity_type);
  $info = $entity_info['field replacement'][$field_name];
  $instance = field_info_instance($entity_type, $info['field']['field_name'], $bundle_name);

  $form['#entity_type'] = $entity_type;
  $form['#bundle'] = $bundle_name;
  $form['#field_name'] = $field_name;

  $form['enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Replace %field with a field instance.', array('%field' => $field_name)),
    '#description' => t('If this is enabled the %field legacy field will be replaced with a regular field and will disappear from the <em>Manage fields</em> page. It will get back if the replacing field instance is deleted.', array('%field' => $field_name)),
    '#default_value' => !empty($instance),
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save settings'));
  return $form;
}

/**
 * Process field replacement form subissions.
 */
function title_field_replacement_form_submit($form, &$form_state) {
  if (title_toggle_field_replacement($form['#entity_type'], $form['#bundle'], $form['#field_name'])) {
    drupal_set_message(t('%field replaced with a field instance.', array('%field' => $form['#field_name'])));
  }
  else {
    drupal_set_message(t('Field replacement removed.'));
  }
  $form_state['redirect'] = _field_ui_bundle_admin_path($form['#entity_type'], $form['#bundle']) . '/fields';
}

/**
 * Toggle field replacement for the given field.
 *
 * @param $entity_type
 *   The name of the entity type.
 * @param $bundle
 *   The bundle the legacy field belongs to.
 * @param $legacy_field
 *   The name of the legacy field to be replaced.
 */
function title_toggle_field_replacement($entity_type, $bundle, $legacy_field) {
  $info = title_field_replacement_info($entity_type, $legacy_field);

  if (!$info) {
    return;
  }

  $field_name = $info['field']['field_name'];
  $instance = field_info_instance($entity_type, $field_name, $bundle);

  if (empty($instance)) {
    $field = field_info_field($field_name);
    if (empty($field)) {
      field_create_field($info['field']);
    }
    $info['instance']['entity_type'] = $entity_type;
    $info['instance']['bundle'] = $bundle;
    field_create_instance($info['instance']);
    return TRUE;
  }
  else {
    field_delete_instance($instance);
    return FALSE;
  }
}
