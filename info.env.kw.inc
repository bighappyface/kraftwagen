<?php

/**
 * @file
 * This file contains the functions that are required to execute
 * `drush kw-env-info`.
 */

/**
 * Implements drush_COMMAND_init() for `drush kw-env-info`.
 */
function drush_kw_env_info_init() {
  kraftwagen_context_init_kraftwagen_root();
}

/**
 * Implements drush_COMMAND for `drush kw-env-info`.
 */
function drush_kraftwagen_kw_env_info() {
  // TODO: Replace with something good.
  $profile_name = kraftwagen_context_get_profile_name();
  return array(
    'profile_name' => $profile_name
  );
}
