Questions CAPTCHA module for Drupal 7
=====================================

## Description 

I wrote this module because I'd had enough of the comment spam on my personal website and I wanted to try something a bit different.

This module adds a CAPTCHA form element to comment forms offering the user a set of 'simple questions that he/she can check as TRUE or FALSE. Really they are statements rather than questions. It also adds a hidden honeypot field which has a configurable name. The module provides an a settings page and pages to view CAPTCHA failures and their IP addresses. 

To let particular user roles bypass the CAPTCHA, give them the 'bypass CAPTCHA' permission.

You can choose to display any number from a set of ten pre-written questions and add or use your own or  via an api hook. As CAPTCHAs go, it is pretty basic but from my experience it will still block most comment spammers and it is something different.

But, it works, is a bit of fun and gives some Drupal APIs a bit of a workout. See it in action on my own site; [http://www.klausharris.de](http://www.klausharris.de) , look at any page where there is a comment form.

## Demo

[http://klausharris.de/comment/reply/342#comment-form](http://klausharris.de/comment/reply/342#comment-form)


## Screenshots

[http://klausharris.de/code/questionscaptcha](http://klausharris.de/code/questionscaptcha)

## Requirements

Drupal 7

## Installation instructions

Install to the modules directory, usually this is sites/all/modules, then configure as you wish. It installs in the Spam control modules group, and provides, help, permissions and admin interface pages. Administrators will by default not see the CAPTCHA.

## Todo

1. A bit of extra functionality is needed when viewing failed attempts plus clearing out the list of
failed attempts, once this is done, I'll push it as a proper project on Drupal.org
2. Possibly write it as a plugin for the Drupal CAPTCHA module.
3. Possibly make it work for more forms such as the contact form.
4. Add to Drupal.org - in progress


## Related Drupal 7 CAPTCHA modules

[https://drupal.org/project/captcha](https://drupal.org/project/captcha)
[https://drupal.org/project/mollom](https://drupal.org/project/mollom)
[https://drupal.org/project/antispam](https://drupal.org/project/antispam)
[https://drupal.org/project/recaptcha](https://drupal.org/project/recaptcha)
[https://drupal.org/project/honeypot](https://drupal.org/project/honeypot)

