<?php
/**
 * @file
 *
 * API Functionality for QuestionsCAPTCHA module
 */

/**
 * @addtogroup hooks
 * @{
 */


/**
 * Alter the honeypot protections added to a particular form. Don't forget to clear the hook cache for this to take
 * effect. You can see the questions in the configuration page here:
 *
 * /admin/config/content/questionscaptcha
 *
 * @see QuestionsCaptcha::getDefaultQuestions(); which is where this alter hook is called.
 * @param array $aOut
 *   Array of questions and TRUE FALSE statements such as:
 * @code
 * $aOut = array(
 *  array(
 *   'key' => 1,
 *   'question' => t('Cats have four legs'),
 *   'answer' => TRUE
 *  ),
 *  array(
 *   'key' => 2,
 *   'question' => t('Cows can drive cars'),
 *   'answer' => FALSE
 *  ),
 * ..
 * @endcode
 */
function hook_questionscaptcha_question_checkboxes_alter(&$aOut) {

  // Alter the $aOut variable as you wish but preserve the array format.

}

/**
 * @} End of "addtogroup hooks".
 */
