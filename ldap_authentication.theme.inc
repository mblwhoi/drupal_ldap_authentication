<?php

/**
 * @file
 * Themes for ldap_authentication module.
 */

//////////////////////////////////////////////////////////////////////////////
// Theme callbacks

/**
 * Theme function for the admin list form.
 */
function theme_ldap_authentication_admin_list($form) {
  drupal_add_tabledrag('ldap_authentication-list-table', 'order', 'sibling', 'server-weight');

  $header = array(
    '',
    t('Server'),
    array('data' => t('Operations'), 'colspan' => 3),
    t('Weight'),
  );

  $rows = array();
  foreach ($form['list'] as $name => $element) {

    if (! preg_match('/^#/', $name ) ) {
      $row = array('');
      $row[] = $element['name']['#value'];
      $row[] = l(t('edit'), 'admin/settings/ldap/ldap_authentication/edit/'. $name);
      $row[] = l($element['status']['#value'] ? t('de-activate') : t('activate'), 'admin/settings/ldap/ldap_authentication/'. ($element['status']['#value'] ? 'deactivate' : 'activate') .'/'. $name);
      $row[] = l(t('delete'), 'admin/settings/ldap/ldap_authentication/delete/'. $name);
      $element['weight']['#attributes']['class'] = "server-weight";
      $row[] = drupal_render($element['weight']);
      $rows[] = array('data' => $row, 'class' => 'draggable'. ($element['status']['#value'] ? ' menu-enabled' : ' menu-disabled'));
      unset($form['list'][$name]);
    }
  }

  if (empty($rows)) {
    $rows[] = array(array('data' => t('No servers defined.'), 'colspan' => 5));
    array_pop($header);
    unset($form['submit']);
  }

  $form['list']['table'] = array('#value' => theme('table', $header, $rows, array('id' => 'ldap_authentication-list-table')));
  return drupal_render($form);
}

