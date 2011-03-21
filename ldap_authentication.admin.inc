<?php

/**
 * @file
 * Module admin page callbacks.
 */

//////////////////////////////////////////////////////////////////////////////
// ldap_authentication settings

/**
 * Implements the settings page.
 *
 * @return
 *   The form structure.
 */
function ldap_authentication_admin_settings() {
  $options_login_process = array(
    LDAP_AUTHENTICATION_AUTH_MIXED => t('Mixed mode. The LDAP authentication is performed only if Drupal authentication fails'),
    LDAP_AUTHENTICATION_AUTH_EXCLUSIVED => t('LDAP directory only')
  );
  $options_login_conflict = array(
    LDAP_AUTHENTICATION_CONFLICT_LOG => t('Disallow login and log the conflict'),
    LDAP_AUTHENTICATION_CONFLICT_RESOLVE => t('Associate local account with the LDAP entry')
  );

  $form['system-options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Authentication mode'),
    '#description' => t('<strong>NOTE:</strong> These settings have no effect on Drupal user with uid 1. The admin account never uses LDAP.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['system-options']['ldap_authentication_login_process'] = array(
    '#type' => 'radios',
    '#title' => t('Choose authentication mode'),
    '#description' => t('Pick the mode based on the types of user accounts and other configuration decisions. If <i>LDAP directory only</i> option is activated some UI modications will be applied.'),
    '#default_value' => variable_get('ldap_authentication_login_process', LDAP_AUTHENTICATION_AUTH_MIXED),
    '#options' => $options_login_process,
    '#required' => TRUE,
  );
  $form['system-options']['ldap_authentication_login_conflict'] = array(
    '#type' => 'radios',
    '#title' => t('Choose user conflict resolve procedure'),
    '#description' => t('Pick what should be done if the local Drupal account already exists with the same login name.'),
    '#default_value' => variable_get('ldap_authentication_login_conflict', LDAP_AUTHENTICATION_CONFLICT_LOG),
    '#options' => $options_login_conflict,
    '#required' => TRUE,
  );

  $form['security-options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Security Options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['security-options']['ldap_authentication_forget_passwords'] = array(
    '#type' => 'checkbox',
    '#title' => t('Do not store users\' passwords during sessions'),
    '#default_value' => variable_get('ldap_authentication_forget_passwords', TRUE),
    '#description' => t('<p>If you use the <strong>ldapdata</strong> module and want to allow users to modify their LDAP attributes, you have two options:</p><ul><li>Setup a special ldap manager DN that has (limited) permissions to edit the requisite LDAP records - using this method means Drupal\'s built in password reset will work;</li> <li>or allow this module to store the user\'s LDAP password, in clear text, during the session;</li></ul><p>Physically, these passwords are stored in the Drupal\'s session table in clear text. This is not ideal and is not the recomended configuration.</p><p>Unless you need to use the latter configuration, leave this checked.</p>'),
  );
  $form['ldap-ui'] = array(
    '#type' => 'fieldset',
    '#title' => t('LDAP UI Options'),
    '#description' => t('<p>Alters LDAP users\' interface only, though admin accounts can still access email and password fields of LDAP users regardless of selections. Does not effect non-LDAP authenticated accounts. </p>'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['ldap-ui']['ldap_authentication_disable_pass_change'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remove password change fields from user edit form'),
    '#default_value' => variable_get('ldap_authentication_disable_pass_change', FALSE),
    '#description' => t('<strong>NOTE:</strong> Request new password feature will be disabled for all users even for the user with uid 1.'),
  );
  $options_email_field = array(
    LDAP_AUTHENTICATION_EMAIL_FIELD_NO => t('Do nothing'),
    LDAP_AUTHENTICATION_EMAIL_FIELD_REMOVE => t('Remove email field from form'),
    LDAP_AUTHENTICATION_EMAIL_FIELD_DISABLE => t('Disable email field on form'),
  );
  $form['ldap-ui']['ldap_authentication_alter_email_field'] = array(
    '#type' => 'radios',
    '#title' => t('Alter email field on user edit form'),
    '#description' => t('Remove or disable email field from user edit form for LDAP authenticated users.'),
    '#default_value' => variable_get('ldap_authentication_alter_email_field', LDAP_AUTHENTICATION_EMAIL_FIELD_NO),
    '#options' => $options_email_field,
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Save configuration'),
  );
  $form['reset'] = array(
    '#type'  => 'submit',
    '#value' => t('Reset to defaults'),
  );

  return $form;
}

/**
 * Submit hook for the settings form.
 */
function ldap_authentication_admin_settings_submit($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):
      variable_set('ldap_authentication_login_process', $values['ldap_authentication_login_process']);
      variable_set('ldap_authentication_login_conflict', $values['ldap_authentication_login_conflict']);
      variable_set('ldap_authentication_forget_passwords', $values['ldap_authentication_forget_passwords']);
      variable_set('ldap_authentication_sync_passwords', $values['ldap_authentication_sync_passwords']);
      variable_set('ldap_authentication_disable_pass_change', $values['ldap_authentication_disable_pass_change']);
      variable_set('ldap_authentication_alter_email_field', $values['ldap_authentication_alter_email_field']);

      drupal_set_message(t('The configuration options have been saved.'));
      break;
    case t('Reset to defaults'):
      variable_del('ldap_authentication_login_process');
      variable_del('ldap_authentication_login_conflict');
      variable_del('ldap_authentication_forget_passwords');
      variable_del('ldap_authentication_sync_passwords');
      variable_del('ldap_authentication_disable_pass_change');
      variable_del('ldap_authentication_alter_email_field');

      drupal_set_message(t('The configuration options have been reset to their default values.'));
      break;
  }

  // Rebuild the menu router.
  menu_rebuild();
}

/**
 * Implements the LDAP servers list.
 *
 * @return
 *   The HTML table with the servers list.
 */
function ldap_authentication_admin_list() {

  // Load ctools export.
  ctools_include('export');

  // Load ldap configs via ctools.  This will get objects both in code and in the db.
  $ldap_configs = ctools_export_crud_load_all('ldap_authentication');

  // Initialize display list.
  $form['list'] = array();

  // For each ldap_config...
  foreach ($ldap_configs as $ldap_config){

    $form['list'][$ldap_config->name] = array(
      'name' => array('#value' => $ldap_config->name),
      'status' => array('#value' => $ldap_config->status),
      'weight' => array(
        '#type' => 'weight',
        '#name' => 'name_'. $ldap_config->name .'_weight',
        '#delta' => 10,
        '#default_value' => $ldap_config->weight,
      ),
    );
  }

  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}


/**
 * Submit hook for the ldap_configs list form.
 */
function ldap_authentication_admin_list_submit($form, &$form_state) {

  $op = $form_state['clicked_button']['#value'];
  switch ($op) {
    case t('Save'):


      // Load ctools export.
      ctools_include('export');


      // For each value...
      foreach ($form_state['clicked_button']['#post'] as $input_name => $val) {

        // If it was a ldap_config...
        if (preg_match('/^name_.*_weight/', $input_name)) {

          // Get the ldap_config name.
          $name = preg_replace(array('/^name_/', '/_weight$/'), array('', ''), $input_name);

          // Load the corresponding ldap ldap_config via ctools
          $ldap_config = ctools_export_crud_load('ldap_authentication', $name);

          // If we could get the ldap_config...
          if ($ldap_config){

            // Update the ldap_config.
            $ldap_config->weight = $val;
            ctools_export_crud_save('ldap_authentication', $ldap_config);
          }
        }
      }
      break;
  }
}

/**
 * Implements the LDAP ldap_config edit page.
 *
 * @param $form_state
 *   A form state array.
 * @param $op
 *   An operatin - add or edit.
 * @param $name
 *   A LDAP ldap_config name.
 *
 * @return
 *   The form structure.
 */
function ldap_authentication_admin_form(&$form_state, $op = NULL, $name = NULL) {

  // Load ctools export.
  ctools_include('export');


  // Initialize server settings form set.
  $form['server-settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Server settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  // Name.
  $form['server-settings']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#description' => t('Choose a <em><strong>unique</strong></em> name for this server configuration.'),
    '#size' => 50,
    '#maxlength' => 255,
    '#required' => TRUE,
  );


  // If editing and a name was given...
  if ($op == "edit" && $name) {

    // Load the corresponding server.
    $edit = ctools_export_crud_load('ldap_authentication', $name);

    // If we couldn't load the name, show an error.
    if (! $edit){
      form_set_error('name', t('Could not load LDAP config for name \'%name\'', array('%name' => $name)));
    }
    else{

      // Set and disable the name field.
      $form['server-settings']['name']['#default_value'] = $edit->name;
      $form['server-settings']['name']['#value'] = $edit->name;
      $form['server-settings']['name']['#disabled'] = TRUE;
      $form['server-settings']['name']['#required'] = FALSE;

    }

  }

  // Otherwise if creating a new server...
  else {

    // Initialize a new server object.
    $edit = new stdClass();
    $edit->name = '';
    $edit->server = '';
    $edit->port = '389';
    $edit->tls = 0;
    $edit->encrypted = 0;
    $edit->basedn = '';
    $edit->user_attr = variable_get('ldap_authentication_default_user_attr', 'uid');
    $edit->mail_attr = variable_get('ldap_authentication_default_mail_attr', 'mail');
    $edit->binddn = '';
    $edit->bindpw = FALSE;
    $edit->login_php = '';
    $edit->filter_php = '';

    // Create a hidden element to indicate that the server config is new.
    $form['server-settings']['is_new'] = array(
                                               '#type' => 'hidden',
                                               '#value' => TRUE
                                               );

  }

  // Set the form fields.
  $form['server-settings']['server'] = array(
    '#type' => 'textfield',
    '#title' => t('LDAP server'),
    '#default_value' => $edit->server,
    '#size' => 50,
    '#maxlength' => 255,
    '#description' => t('The domain name or IP address of your LDAP Server.'),
    '#required' => TRUE,
  );
  $form['server-settings']['port'] = array(
    '#type' => 'textfield',
    '#title' => t('LDAP port'),
    '#default_value' => $edit->port,
    '#size' => 5,
    '#maxlength' => 5,
    '#description' => t('The TCP/IP port on the above server which accepts LDAP connections. Must be an integer.'),
  );
  $form['server-settings']['tls'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use Start-TLS'),
    '#default_value' => $edit->tls,
    '#description' => t('Secure the connection between the Drupal and the LDAP servers using TLS.<br /><em>Note: To use START-TLS, you must set the LDAP Port to 389.</em>'),
  );
  $form['server-settings']['encrypted'] = array(
    '#type' => 'checkbox',
    '#title' => t('Store passwords in encrypted form'),
    '#default_value' => $edit->encrypted,
    '#description' => t('Secure the password in LDAP by storing it MD5 encrypted (use with care, as some LDAP directories may do this automatically, what would cause logins problems).'),
  );

  $form['login-procedure'] = array(
    '#type' => 'fieldset',
    '#title' => t('Login procedure'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['login-procedure']['basedn'] = array(
    '#type' => 'textarea',
    '#title' => t('Base DNs'),
    '#default_value' => $edit->basedn,
    '#cols' => 50,
    '#rows' => 6,
    '#description' => t('Base DNs for users. Enter one per line in case you need several of them.'),
  );
  $form['login-procedure']['user_attr'] = array(
    '#type' => 'textfield',
    '#title' => t('UserName attribute'),
    '#default_value' => $edit->user_attr,
    '#size' => 30,
    '#maxlength' => 255,
    '#description' => t('The attribute that holds the users\' login name. (eg. <em style="font-style: normal; padding: 1px 3px; border: 1px solid #8888CC; background-color: #DDDDFF">cn</em> for eDir or <em style="font-style: normal; padding: 1px 3px; border: 1px solid #8888CC; background-color: #DDDDFF">sAMAccountName</em> for Active Directory).'),
  );
  $form['login-procedure']['mail_attr'] = array(
    '#type' => 'textfield',
    '#title' => t('Email attribute'),
    '#default_value' => $edit->mail_attr,
    '#size' => 30,
    '#maxlength' => 255,
    '#description' => t('The attribute that holds the users\' email address. (eg. <em style="font-style: normal; padding: 1px 3px; border: 1px solid #8888CC; background-color: #DDDDFF">mail</em>).'),
  );
  $form['login-procedure']['login_php'] = array(
    '#type' => 'textarea',
    '#title' => t('PHP to transform login name'),
    '#default_value' => $edit->login_php,
    '#cols' => 25,
    '#rows' => 5,
    '#description' => t('Enter PHP to transform login name before it is sent to LDAP for authentication. Careful, bad PHP code here will break your site. If left empty, no name transformation will be done. Change following example code to enable transformation:<br /><code>return $name;</code>'),
  );
  $form['login-procedure']['filter_php'] = array(
    '#type' => 'textarea',
    '#title' => t('PHP to filter users based on their LDAP data'),
    '#default_value' => $edit->filter_php,
    '#cols' => 25,
    '#rows' => 5,
    '#description' => t('Enter PHP to filter users which are allowed to login based on their LDAP data. Careful, bad PHP code here will break your site. If left empty, no filtering will be done. The code should return TRUE to allow authentication. Following example shows how to disallow users without their homeDirectory set:<br /><code>return isset($ldap[\'homeDirectory\']) && isset($ldap[\'homedirectory\'][0]);</code>'),
  );

  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced configuration'),
    '#description' => t('<p>The process of authentication starts by establishing an anonymous connection to the LDAP directory and looking up for the user on it. Once this user is found, LDAP authentication is performed on them.</p><p>However, some LDAP configurations (specially common in <strong>Active Directory</strong> setups) restrict anonymous searches.</p><p>If your LDAP setup does not allow anonymous searches, or these are restricted in such a way that login names for users cannot be retrieved as a result of them, then you have to specify here a DN//password pair that will be used for these searches.</p><p>For security reasons, this pair should belong to an LDAP account with stripped down permissions.</p><p>"Test" tries authentication with the saved DN and password and prints the result.</p>'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  $form['advanced']['binddn'] =  array(
    '#type' => 'textfield',
    '#title' => t('DN for non-anonymous search'),
    '#default_value' => $edit->binddn,
    '#size' => 50,
    '#maxlength' => 255,
  );
  if (!$edit->bindpw) {
    $form['advanced']['bindpw'] = array(
      '#type' => 'password',
      '#title' => t('Password for non-anonymous search'),
      '#size' => 12,
      '#maxlength' => 255,
    );
  }
  else {
    // Give an option to clear the password.
    $form['advanced']['bindpw_clear'] = array(
      '#type' => 'checkbox',
      '#title' => t('Clear current password'),
      '#default_value' => FALSE,
    );
  }
  $form['advanced']['test'] = array(
    '#type'  => 'submit',
    '#value' => t('Test'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );

  return $form;
}

/**
 * Validate hook for the LDAP config form.
 */
function ldap_authentication_admin_form_validate($form, &$form_state) {
  $values = $form_state['values'];

  // If the name is set...
  if (isset($values['name'])){

    // If the ldap_config is new...
    if (isset($values['is_new']) && $values['is_new'] ) {

      // Load ctools export.
      ctools_include('export');

      // If a ldap_config already exists with the name..
      $ldap_config = ctools_export_crud_load('ldap_authentication', $values['name']);
      if($ldap_config){
        form_set_error('name', t('An LDAP config with the  name %name already exists.', array('%name' => $values['name'])));
      }
    }
  }

  // Otherwise if a name was not set...
  else{
    form_set_error('name', 'You must provide a unique name for this LDAP config.');
  }


  // If the port was not numeric...
  if (!is_numeric($values['port'])) {
    form_set_error('port', t('The TCP/IP port must be an integer.'));
  }
}

/**
 * Submit hook for the LDAP ldap_config form.
 */
function ldap_authentication_admin_form_submit($form, &$form_state) {
  $op = $form_state['clicked_button']['#value'];
  $values = $form_state['values'];
  switch ($op) {
    case t('Save configuration'):

      // Load ctools export.
      ctools_include('export');

      // Try to load the ldap_config config.
      $ldap_config = ctools_export_crud_load('ldap_authentication', $values['name']);

      // If the ldap_config config does not yet exist...
      if (! $ldap_config) {

        // Create a ldap_config object.
        $ldap_config = new stdClass();
        $ldap_config->name = $values['name'];
        $ldap_config->status = 1;
        $ldap_config->server = $values['server'];
        $ldap_config->port = $values['port'];
        $ldap_config->tls = $values['tls'];
        $ldap_config->encrypted = $values['encrypted'];
        $ldap_config->basedn = $values['basedn'];
        $ldap_config->user_attr = trim($values['user_attr']);
        $ldap_config->mail_attr = trim($values['mail_attr']);
        $ldap_config->binddn = $values['binddn'];
        $ldap_config->bindpw = $values['bindpw'];
        $ldap_config->login_php = trim($values['login_php']);
        $ldap_config->filter_php = trim($values['filter_php']);

        // Save the ldap_config.
        ctools_export_crud_save('ldap_authentication', $ldap_config);
        drupal_set_message(t('LDAP configuration %name has been added.', array('%name' => $values['name'])));
        watchdog('ldap_authentication', 'LDAP configuration %name has been added.', array('%name' => $values['name']));
      }

      // Otherwise if the ldap_config already exists...
      else {

        // Update the ldap_config.
        $ldap_config->server = $values['server'];
        $ldap_config->port = $values['port'];
        $ldap_config->tls = $values['tls'];
        $ldap_config->encrypted = $values['encrypted'];
        $ldap_config->basedn = $values['basedn'];
        $ldap_config->user_attr = trim($values['user_attr']);
        $ldap_config->mail_attr = trim($values['mail_attr']);
        $ldap_config->binddn = $values['binddn'];
        $ldap_config->bindpw = $values['bindpw'];
        $ldap_config->login_php = trim($values['login_php']);
        $ldap_config->filter_php = trim($values['filter_php']);

        // If bindpw_clear is set...
        if (isset($values['bindpw_clear'])) {

          // Clear the ldap_config's bindpw attribute;
          if ($values['bindpw_clear']) {
            $ldap_config->bindpw = '';
          }
        }

        // Save the ldap_config.
        ctools_export_crud_save('ldap_authentication', $ldap_config);
        drupal_set_message(t('LDAP Configuration %name has been updated.', array('%name' => $values['name'])));
        watchdog('ldap_authentication', 'LDAP Configuration %name has been updated.', array('%name' => $values['name']));
      }

      $form_state['redirect'] = 'admin/settings/ldap/ldap_authentication/list';
      break;
    case t('Test'):
      global $_ldap_authentication_ldap;
      if (isset($values['name']) && _ldap_authentication_init($values['name'])) {
        // Try to authenticate.
        $dn = $_ldap_authentication_ldap->getOption('binddn');
        $pass = $_ldap_authentication_ldap->getOption('bindpw');
        if (!$_ldap_authentication_ldap->connect($dn, $pass)) {
          drupal_set_message(t('Authentication with the LDAP server for the dn %dn and saved password failed.', array('%dn' => $dn)), 'error');
        }
        else {
          drupal_set_message(t('Authentication with the LDAP server for the dn %dn and saved password succeeded.', array('%dn' => $dn)));
        }
      }
      else {
        drupal_set_message(t('Cannot load server settings. Please save configuration first.'), 'error');
      }
      break;
  }
}

/**
 * De-activates the LDAP ldap_config.
 *
 * @param $name
 *   A LDAP ldap_config ID.
 *
 * @return
 */
function ldap_authentication_admin_deactivate($name) {

  // Load ctools export.
  ctools_include('export');

  // Try to load the ldap_config config.
  $ldap_config = ctools_export_crud_load('ldap_authentication', $name);

  if ($ldap_config) {

    // Set the status to 0.
    $ldap_config->status = 0;

    // Save the ldap_config.
    ctools_export_crud_save('ldap_authentication', $ldap_config);

    drupal_set_message(t('LDAP Configuration %name has been de-activated.', array('%name' => $ldap_config->name)));
    watchdog('ldap_authentication', 'LDAP ldap_config %name was de-activated.', array('%name' => $ldap_config->name));
  }

  drupal_goto('admin/settings/ldap/ldap_authentication/list');
}

/**
 * Activates the LDAP ldap_config.
 *
 * @param $name
 *   A LDAP ldap_config name.
 *
 * @return
 */
function ldap_authentication_admin_activate($name) {

  // Load ctools export.
  ctools_include('export');

  // Try to load the ldap_config config.
  $ldap_config = ctools_export_crud_load('ldap_authentication', $name);

  if ($ldap_config) {

    // Set the status to 1.
    $ldap_config->status = 1;

    // Save the ldap_config.
    ctools_export_crud_save('ldap_authentication', $ldap_config);

    drupal_set_message(t('LDAP Configuration %name has been activated.', array('%name' => $ldap_config->name)));
    watchdog('ldap_authentication', 'LDAP ldap_config %name was activated.', array('%name' => $ldap_config->name));
  }
  drupal_goto('admin/settings/ldap/ldap_authentication/list');
}

/**
 * Implements the LDAP ldap_config delete page.
 *
 * @param $form_state
 *   A form state array.
 * @param $sid
 *   A LDAP ldap_config name.
 *
 * @return
 *   The form structure.
 */
function ldap_authentication_admin_delete(&$form_state, $name) {

  // Load ctools export.
  ctools_include('export');

  // Try to load the ldap_config config.
  $ldap_config = ctools_export_crud_load('ldap_authentication', $name);


  // If there was a ldap_config...
  if ($ldap_config) {

    $form['name'] = array(
      '#type' => 'hidden',
      '#value' => $ldap_config->name,
    );
    return confirm_form(
      $form,
      t('Are you sure you want to delete the LDAP ldap_config named <em><strong>%name</strong></em>?', array('%name' => $ldap_config->name)),
      'admin/settings/ldap/ldap_authentication/list',
      t('<p>This action cannot be undone.</p>'),
      t('Delete'),
      t('Cancel')
    );
  }
  drupal_goto('admin/settings/ldap/ldap_authentication/list');
}

/**
 * Submit hook for the LDAP ldap_config delete page.
 */
function ldap_authentication_admin_delete_submit($form, &$form_state) {
  $values = $form_state['values'];

  if ($values['confirm'] && $values['name']) {

    // Load ctools export.
    ctools_include('export');

    // Try to load the ldap_config config.
    $ldap_config = ctools_export_crud_load('ldap_authentication', $values['name']);

    // If there was a ldap_config...
    if($ldap_config){

      // Delete it.
      ctools_export_crud_delete('ldap_authentication', $ldap_config->name);
      drupal_set_message(t('LDAP Configuration %name has been deleted.', array('%name' => $values['name'])));
      watchdog('ldap_authentication', 'LDAP Configuration %name has been deleted.', array('%name' => $values['name']));
    }
  }
}

/**
 * Implements the LDAP admin  page.
 *
 * @return
 *   The themed HTML page.
 */
function ldap_authentication_admin_menu_block_page() {
  return theme('admin_block_content', system_admin_menu_block(menu_get_item()));
}

