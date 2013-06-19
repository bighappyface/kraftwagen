<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-init-database`.
 */

/**
 * Implements drush_COMMAND_init() for `drush kw-init-database`.
 */
function drush_kw_init_database_init() {
  kraftwagen_context_init_kraftwagen_root();
}

/**
 * Implements drush_COMMAND() for `drush kw-init-database`.
 */
function drush_kraftwagen_kw_init_database() {
  $profile_name = kraftwagen_context_get_profile_name();
  drush_invoke_process('@self', 'site-install', array($profile_name, '--yes'));
}
