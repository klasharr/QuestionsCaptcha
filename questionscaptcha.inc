<?php
/**
 * @file
 * questionscaptcha.inc
 *
 * Contains among others form functions and page callbacks
 */


/**
 * Form callback for the settings page configuration form.
 *
 * @param $form
 * @param $form_state
 * @return array
 */
function _form_questionscaptcha_settings($form, &$form_state) {

  $form['questionscaptcha_settings_help'] = array(
    '#markup' => t('See also: !permissions_link !help_link.' ,
      array(
        '!permissions_link' => l(t('permissions page'), '/admin/people/permissions'),
        '!help_link' => module_exists('help') ?
          ', '. l(t('help page'), 'admin/help/questionscaptcha') :
          t(', and enable the Help module to see the help page.'),
      )
    ),
    '#prefix' => '<p>',
    '#suffix' => '</p>',
  );

  $form['questionscaptcha_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Questions CAPTCHA'),
    '#weight' => 0,
  );

  $aQuestionsCntOptions = array();
  // We'll allow a maximum of ten questions. In reality, you would probably display no more than three.
  for ($i = 2; $i < 10; $i++) {
    $aQuestionsCntOptions[$i] = $i;
  }

  $form['questionscaptcha_settings']['questionscaptcha_number_to_show'] = array(
    '#type' => 'select',
    '#options' => $aQuestionsCntOptions,
    '#required' => TRUE,
    '#title' => t('Questions count'),
    '#default_value' => variable_get('questionscaptcha_number_to_show', 5),
    '#description' => t('How questions do you want to show in the CAPTCHA? Less than five is best. The questions are shown ' .
    'in a randomised list.'),
    '#weight' => 15,
  );

  $form['questionscaptcha_settings']['questionscaptcha_choose_defaults'] = array(
    '#type' => 'textarea',
    '#rows' => 8,
    '#value' => QuestionsCaptcha::getQuestionsAndAnswersAsText(),
    '#disabled' => TRUE,
    '#title' => t('Default questions'),
    '#description' => t('These are the default questions that will be used in the selection shown to the user.'),
    '#weight' => 20,
  );

  $form['questionscaptcha_settings']['questionscaptcha_own_questions'] = array(
    '#title' => t('Or use your questions of your own'),
    '#type' => 'textarea',
    '#rows' => 5,
    '#default_value' => variable_get('questionscaptcha_own_questions', ''),
    '#description' => t('If you enter questions here, they will be used instead of the module defaults above.').
    '<br/><br/>'.t('Write them in this format:').'<br/><br/>'.
    t('TRUE|The sun is hot').'<br/>'.
    t('FALSE|Soap is good to eat'),
    '#weight' => 30,
  );

  $form['questionscaptcha_advanced_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Questions CAPTCHA Advanced settings'),
    '#weight' => 10,
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['questionscaptcha_advanced_settings']['questionscaptcha_checkbox_name_salt'] = array(
    '#type' => 'textfield',
    '#maxlength' => 30,
    '#size' => 30,
    '#default_value' => variable_get('questionscaptcha_checkbox_name_salt', ''),
    '#title' => t('Salt for questions CAPTCHA element name hash'),
    '#description' => t('To confuse any spam bots further, we change the form field element ID on every form cycle. Add a value here to salt the randomising hash. It\'s optional.'),
  );

  $form['questionscaptcha_advanced_settings']['questionscaptcha_cache_method'] = array(
    '#type' => 'select',
    '#required' => TRUE,
    '#options' => array('session' => 'Session', 'drupal_cache' => 'Drupal cache'),
    '#default_value' => variable_get('questionscaptcha_cache_method', 'session'),
    '#title' => t('Cache method for questions'),
    '#description' => t('Normally just leave this to <em>Session</em>, but if your site is behind Varnish ' .
    'or a similar cache, use <em>Drupal cache</em> instead. With this method no PHP session is needed. ' .
    'The trade off is that the questions and form element ID name are only randomised on page cache ' .
    'refreshes which might take a number of hours.'),
  );

  $form['questionscaptcha_advanced_settings']['clear_session_cache'] = array(
    '#type' => 'submit',
    '#value' => t('Clear session cache'),
    '#ajax' => array(
      'callback' => '_ajax_questionscaptcha_session_clear_callback',
      'wrapper' => 'questionscaptcha_callback_div',
    ),
    '#description' => t('This is useful if you want to see how your questions are randomised on the comment form.'),
  );

  // Empty container div that gets filled with a message from the above ajax callback.
  $form['questionscaptcha_advanced_settings']['sessionmessage'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="questionscaptcha_callback_div">',
    '#suffix' => '</div>',
  );

  $form['questionscaptcha_advanced_settings']['questionscaptcha_watchdog_on_fail'] = array(
    '#type' => 'radios',
    '#required' => TRUE,
    '#options' => array(
      1 => t('Yes'),
      0 => t('No')
    ),
    '#default_value' => variable_get('questionscaptcha_watchdog_on_fail', 'session'),
    '#title' => t('Record CAPTCHA fails in watchdog'),
  );

  $form['questionscaptcha_advanced_settings']['questionscaptcha_log_reject_ipaddress'] = array(
    '#type' => 'radios',
    '#required' => TRUE,
    '#options' => array(
      1 => t('Yes'),
      0 => t('No')
    ),
    '#default_value' => variable_get('questionscaptcha_log_reject_ipaddress', 0),
    '#title' => t('Record CAPTCHA fail IP addresses in the database.'),
  );

  $form['questionscaptcha_advanced_settings']['questionscaptcha_honeypot'] = array(
    '#type' => 'textfield',
    '#maxlength' => 30,
    '#size' => 30,
    '#default_value' => variable_get('questionscaptcha_honeypot', 'url'),
    '#title' => t('Name of the honeypot field'),
    '#description' => t('The honeypot field is an invisible form field that spam bots will try and fill in but which '.
      'remain empty.'),
  );

  return system_settings_form($form);
}


/**
 * Validation callback for the CAPTCHA part of the comment form.
 *
 * @see questionscaptcha_form_alter()
 * @param $form
 * @param $form_state
 */
function _form_questionscaptcha_captcha_validate(&$form, &$form_state) {
  global $user;

  if($aErrors = _questionscaptcha_get_form_errors($form_state)){
    $aVariables = array(
      'name' => $form_state['values']['name'],
      'subject' => $form_state['values']['subject'],
      'comment_body' => $form_state['values']['comment_body']['und'][0]['value'],
      'ip_address' => ip_address(),
      'user_id' => $user->uid,
      'failures' => $aErrors,
    );

    if(variable_get('questionscaptcha_watchdog_on_fail',1) == 1) {
      QuestionsCaptcha::watchdogLogRejectComment($aVariables);
    }
    if(variable_get('questionscaptcha_log_reject_ipaddress', 0) == 1) {
      QuestionsCaptcha::dbLogRejectIPAddress($aVariables);
    }

    // Allow other modules to respond to a reject
    module_invoke_all('questionscaptcha', $form['#id'], $aVariables);

    // Register a flood event for anonymous users
    flood_register_event('questionscaptcha');
  }
}

/**
 * Determine whether there are errors in the CAPTCHA part of the form. The fail message is kept deliberately vague.
 *
 * @param array $form_state
 * @return array
 */
function _questionscaptcha_get_form_errors($form_state){

  // The checkbox name is configurable from the settings form so we need to grab that.
  $sCheckboxName = QuestionsCaptcha::getCaptchaQuestionsCheckboxFormElementName();
  $aErrors = array();

  if (!QuestionsCaptcha::isValidCheckboxValuesArray($form_state['values'][$sCheckboxName])) {
    form_set_error('', t('There was a problem with your form submission.'));
    $aErrors[] = 'captcha_fail';
  }

  if(!empty($form_state['values'][variable_get('questionscaptcha_honeypot', 'url')])){
    // If we already have a questions captcha error, we don't need to create another duplicate error message.
    if(empty($aErrors)){
      form_set_error('', t('There was a problem with your form submission.'));
    }
    $aErrors[] = 'honeypot_fail';
  }
  if(!empty($aErrors)) return $aErrors;
}

/**
 * Submit callback for questions CAPTCHA part of the comment form.
 *
 * @param array $form
 * @param array $form_state
 */
function _form_questionscaptcha_captcha_submit(&$form, &$form_state) {
  QuestionsCaptcha::clearSessionCache();
}

/**
 * Ajax submit button callback on main settings form
 *
 * @see _form_questionscaptcha_main_settings()
 * @param array $form
 * @param array $form_state
 * @return array
 */
function _ajax_questionscaptcha_session_clear_callback(&$form, $form_state) {
  QuestionsCaptcha::clearSessionCache();
  $element = $form['sessionmessage'];
  $element['#markup'] = t('Session cache cleared');
  return $element;
}

/**
 * @todo cleanup
 * @return array
 */
function _page_questionscaptcha_reject_ip_addresses_list_group(){

  $aHeader = array(
    array(
      'data' => t('IP addresses'),
      'field' => 'ip_address',
      'sort' => 'desc'
    ),
    array(
      'data' => t('Blocked'),
      'field' => 'aggregate'
    ),
  );

  try {

    $aResults = db_query(
      "SELECT ip_address, count(ip_address) as aggregate FROM {questionscaptcharejects} group by ip_address");

    $aRows  = array();
    if(!empty($aResults)) {
      foreach ($aResults as $oRecord) {
        $aRows[] = array_map('check_plain', (array) $oRecord);
      }
    }

    $aBuild['header'] = array(
      '#markup' => l(t("Show all records"), 'admin/reports/ips'),
      '#prefix' => '<p>',
      '#suffix' => '</p>',
    );

    $aBuild['pager_table'] = array(
      '#theme' => 'table',
      '#header' => $aHeader,
      '#rows' => $aRows,
      '#empty' => t('There are no records in the reject IP addresses table yet.'),
    );

    // Attach the pager theme
    $aBuild['pager_pager'] = array('#theme' => 'pager');

    return $aBuild;

  } catch (Exception $e) {
    return array(
      '#markup' => '<p>'.t('There was a problem getting the database records: @message', array('@message' => $e->getMessage())).'</p>',
    );
  }
}

/**
 * Page callback to view rejected CAPTCHA attempt ip addresses.
 *
 * @return array
 */
function _page_questionscaptcha_reject_ip_addresses_list() {

  $aHeader = array(
    array(
      'data' => t('Date time'),
      'field' => 'r.timestamp'
    ),
    array(
      'data' => t('IP addresses'),
      'field' => 'r.ip_address'
    ),
    array(
      'data' => t('Type'),
      'field' => 'r.reject_type'
    ),
  );

  try {
    $query = db_select('questionscaptcharejects', 'r');
    $query->fields('r', array('timestamp', 'ip_address', 'reject_type'));
    $query = $query
      ->extend('TableSort')
      ->extend('PagerDefault')
      ->orderByHeader($aHeader)
      ->limit(15);

    $aResults = $query->execute();
    $aRows  = array();
    if(!empty($aResults)) {
      foreach ($aResults as $oRecord) {
        $oOut = $oRecord;
        $oOut->timestamp = format_date($oRecord->timestamp,  'custom', 'Y-m-d H:i O');
        $oOut = array_map('check_plain', (array) $oOut);
        $aRows[] = $oOut;
      }
    }

    $aBuild['header'] = array(
      '#markup' => l(t("Group by number times blocked"), 'admin/reports/ips/group   '),
      '#prefix' => '<p>',
      '#suffix' => '</p>',
    );

    $aBuild['pager_table'] = array(
      '#theme' => 'table',
      '#header' => $aHeader,
      '#rows' => $aRows,
      '#empty' => t('There are no records in the reject IP addresses table yet.'),
    );

    // Attach the pager theme
    $aBuild['pager_pager'] = array('#theme' => 'pager');

    return $aBuild;

  } catch (Exception $e) {
    return array(
      '#markup' => '<p>'.t('There was a problem getting the database records: @message', array('@message' => $e->getMessage())).'</p>',
    );
  }
}