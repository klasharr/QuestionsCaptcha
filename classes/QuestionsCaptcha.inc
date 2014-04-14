<?php

/**
 * Class QuestionsCaptcha
 *
 * The purpose of this class is to organise functionality around the questions CAPTCHA and isolate
 * code from the procedural style Drupal files.
 */
class QuestionsCaptcha {

  /**
   * This cache key string is used when the cache method is set to Drupal cache.
   * @var string
   */
  private static $sCacheKey = 'questionscaptcha_comment_form_captcha_questions';

  /**
   * Get the default questions to display to the user. There are three actual ways to retrieve the questions shown to
   * the user:
   *
   * 1) from the array defined in this method
   * 2) from a call to self::getCustomQuestions() which returns questions that the user entered in the settings
   *    configuration form
   * @see /admin/config/content/questionscaptcha
   * 3) from a combination of 1) and an API call to drupal_alter which lets other modules alter the array programmatically
   * @see questionscaptcha.api.php
   *
   * @param bool $bReturnModuleDefaults
   * @return array
   */
  public static function getDefaultQuestions($bReturnModuleDefaults = FALSE) {

    if (($bReturnModuleDefaults == FALSE) && ($aCustomQuestions = self::getCustomQuestions())) {
      return $aCustomQuestions;
    }

    $aOut = array(
      array(
        'key' => 1,
        'question' => t('Cats have four legs'),
        'answer' => TRUE
      ),
      array(
        'key' => 2,
        'question' => t('Cows can drive cars'),
        'answer' => FALSE
      ),
      array(
        'key' => 3,
        'question' => t('Most cars have seven wheels'),
        'answer' => FALSE
      ),
      array(
        'key' => 4,
        'question' => t('Standing in the rain is the best way to keep dry'),
        'answer' => FALSE
      ),
      array(
        'key' => 5,
        'question' => t('Hamsters are cute'),
        'answer' => TRUE
      ),
      array(
        'key' => 6,
        'question' => t('The sun rises in the east'),
        'answer' => TRUE
      ),
      array(
        'key' => 7,
        'question' => t('Coffee is something you drink'),
        'answer' => TRUE
      ),
      array(
        'key' => 8,
        'question' => t('Children are generally smaller than adults'),
        'answer' => TRUE
      ),
      array(
        'key' => 9,
        'question' => t('Fish live in the water'),
        'answer' => TRUE
      ),
      array(
        'key' => 10,
        'question' => t('Tomatoes are blue'),
        'answer' => FALSE
      ),
    );

    // Give other modules a chance to alter or add their own questions
    drupal_alter('questionscaptcha_question_checkboxes', $aOut);
    return $aOut;
  }

  /**
   * Get questions the user entered into the admin form
   * @see /admin/config/content/questionscaptcha
   *
   * @return array|bool
   */
  private static function getCustomQuestions() {

    $sQuestions = variable_get('questionscaptcha_own_questions', NULL);
    if (empty($sQuestions)) {
      return FALSE;
    }

    $aLines = explode("\n", $sQuestions);
    $aCustomQuestions = array();
    for ($i = 0; $i < count($aLines); $i++) {
      $aTmp = explode('|', $aLines[$i]);
      $aCustomQuestions[] = array(
        'key' => $i + 1,
        'answer' => ($aTmp[0] == 'TRUE' ? TRUE : FALSE),
        'question' => $aTmp[1],
      );
    }
    return $aCustomQuestions;
  }

  /**
   * Get the CAPTCHA questions
   *
   * @return array
   *   Of questions shuffled, sliced, cached and returned
   */
  private static function getCachedQuestionsPreselectedAndSliced() {

    if ($aQuestions = self::getCachedData()) {
      return $aQuestions;
    }
    $aQuestions = self::getDefaultQuestions();

    shuffle($aQuestions);
    $aOut = array_slice($aQuestions, 0, variable_get('questionscaptcha_number_to_show', 5));
    self::setCachedData($aOut);
    return $aOut;
  }

  /**
   * Get the checkbox data used to build the checkboxes in the CAPTCHA form
   *
   * @see questionscaptcha_form_alter()
   *   which is where this is called
   * @return array
   *   An indexed array whose values are the questions. The index is important as we need that for validation
   *   of the checkbox on submission.
   */
  public static function getCheckBoxElementData() {

    $aQuestions = self::getCachedQuestionsPreselectedAndSliced();
    $aOut = array();
    foreach ($aQuestions as $aValues) {
      $aOut[$aValues['key']] = $aValues['question'];
    }
    return $aOut;
  }

  /**
   * Retrieve the default module questions and answers as text.
   *
   * @see _form_questionscaptcha_main_settings()
   *   which is where this is called
   * @return string
   *   e.g.
   *   TRUE|hamsters are cute
   *   FALSE|water is dry
   *   TRUE|...
   */
  public static function getQuestionsAndAnswersAsText() {

    $sOut = '';
    foreach (self::getDefaultQuestions(TRUE) as $aQuestion) {
      $sOut .= ($aQuestion['answer'] == TRUE ? 'TRUE' : 'FALSE') . ' | ' . $aQuestion['question'] . "\n";
    }
    return $sOut;
  }

  /**
   * Store the form questions data in the cache. This will either be stored in the $_SESSION or drupal cache depending
   * on the setting in the main configuration form.
   *
   * The data stored includes two things:
   *
   * 1) the questions that the user will see
   * 2) the name for the form checkbox element which is also randomised
   *
   * @param $aQuestionsData
   *   Array containing the questions and answers
   */
  private static function setCachedData($aQuestionsData) {

    $sFormElementTitle = substr(
      'question_radios' . md5(date('YmdHi') . variable_get('questionscaptcha_checkbox_name_salt', '')),
      0, 25);

    if (variable_get('questionscaptcha_cache_method', 'session') == 'session') {
      $_SESSION['questionscaptcha']['questions_keypairs'] = $aQuestionsData;
      $_SESSION['questionscaptcha']['questions_form_element_name'] = $sFormElementTitle;
    }
    else {
      cache_set(self::$sCacheKey,
        array(
          'questions_keypairs' => $aQuestionsData,
          'questions_form_element_name' => $sFormElementTitle
        ),
        'cache', CACHE_TEMPORARY
      );
    }
  }

  /**
   * Return the cached data to build the CAPTCHA questions element
   *
   * @param $sType
   *   string detailing which bit of data should be returned, possible values are:
   *   - all
   *   - form_element_name
   *   - questions
   * @return mixed
   *   array|string|boolean depending on the value of $sType
   */
  private static function getCachedData($sType = 'questions') {

    if (variable_get('questionscaptcha_cache_method', 'session') == 'session') {
      if (!empty($_SESSION['questionscaptcha']['questions_keypairs']) &&
        !empty($_SESSION['questionscaptcha']['questions_form_element_name'])
      ) {
        $aCachedData = $_SESSION['questionscaptcha'];
      }
    }
    else {
      $aCacheEntry = cache_get(self::$sCacheKey);
      if (!empty($aCacheEntry->data)) {
        $aCachedData = $aCacheEntry->data;
      }
    }

    if (empty($aCachedData)) {
      return FALSE;
    }

    switch ($sType) {
      case 'all':
        return $aCachedData;
        break;
      case 'form_element_name':
        return $aCachedData['questions_form_element_name'];
        break;
      case 'questions':
        return $aCachedData['questions_keypairs'];
    }
  }

  /**
   * Clear the CAPTCHA questions and form element name from the $_SESSION cache.
   */
  public static function clearSessionCache() {
    unset($_SESSION['questionscaptcha']['questions_keypairs']);
    unset($_SESSION['questionscaptcha']['questions_form_element_title']);
  }

  /**
   * Get the name for the form element containing the CAPTCHA questions checkboxes
   *
   * @return string
   *   The name
   */
  public static function getCaptchaQuestionsCheckboxFormElementName() {

    $sFormElementName = self::getCachedData('form_element_name');
    // Force a cache reset
    if (empty($sFormElementName)) {
      self::getCachedQuestionsPreselectedAndSliced();
    }
    return self::getCachedData('form_element_name');
  }

  /**
   * Validation method to see if the answer and question pair are valid. We basically test the checkbox values
   * entered by the user against our real questions and answers.
   *
   * @param $aValues
   *   Array of values from the checkboxes element to test
   * @return bool
   *   TRUE if valid, FALSE otherwise
   */
  public static function isValidCheckboxValuesArray($aValues) {

    $aRawQuestions = self::getDefaultQuestions();

    foreach ($aValues as $iIndex => $iValue) {
      foreach ($aRawQuestions as $i => $aQuestion) {

        if ($aQuestion['key'] != $iIndex) {
          continue;
        }
        elseif ($aQuestion['answer'] == TRUE && $iValue == $iIndex) {
          continue;
        }
        elseif ($aQuestion['answer'] != TRUE && $iValue != $iIndex) {
          continue;
        }
        elseif ($aQuestion['answer'] == TRUE && $iValue != $iIndex) {
          return FALSE;
        }
        elseif ($aQuestion['answer'] != TRUE && $iValue == $iIndex) {
          return FALSE;
        }
      }
    }
    return TRUE;
  }

  /**
   * Log rejected comments to the database.
   *
   * @param $aVariables
   *   Array containing details about the rejected comment
   */
  public static function dbLogRejectIPAddress($aVariables) {

    try {
      $oNewRecord = array(
        "ip_address" => $aVariables['ip_address'],
        "reject_type" => implode(', ', $aVariables['failures']),
        "timestamp" => time(),
      );
      drupal_write_record('questionscaptcharejects', $oNewRecord);
    } catch (Exception $e) {
      watchdog('Questions CAPTCHA', $e->getMessage(), array(), WATCHDOG_ERROR);
    }
  }

  /**
   * Log rejected comments to watchdog
   *
   * @param $aVariables
   *   Array containing details about the rejected comment
   */
  public static function watchdogLogRejectComment($aVariables) {

    watchdog('Questions CAPTCHA',
      'Failed attempt to bypass captcha. Name: @name<br/>Subject: @subject<br/>' .
      'Comment body: @comment_body Type: @type',
      array(
        '@name' => $aVariables['name'],
        '@subject' => $aVariables['subject'],
        '@comment_body' => $aVariables['comment_body'],
        '@type' => implode(',', $aVariables['failures']),
      ),
      WATCHDOG_NOTICE);
  }
}