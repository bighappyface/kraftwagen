<?php


function drush_kraftwagen_create_project($dir = null) {
  $dir = realpath($dir ? $dir : getcwd());

  // if the dir has a kraftwagenrc file, we should load it
  if (file_exists($dir . DIRECTORY_SEPARATOR . KRAFTWAGEN_RC)) {
    kraftwagen_context_load_config_file($dir . DIRECTORY_SEPARATOR . KRAFTWAGEN_RC);
  }

  // determine source directory
  $dir_src = kraftwagen_context_get_option('src-dir');
  if (is_dir($dir . DIRECTORY_SEPARATOR . $dir_src) && !drush_get_option('force')) {
    drush_log(dt('The directory !dir already exists. Use --force if you really want Kraftwagen to overwrite files in this directory.', array('!dir' => $dir_src)), 'error');
    return;
  }
  
  // ask for the human readable name of the project. This can be used for the
  // name property of the install profile. Defaults to the name of the current
  // directory
  $project_label = drush_prompt(dt('Enter the human readable name of the project'), basename($dir));
  
  // create machine name for the project. This defaults to a simplified version
  // of the human readable name. This is used for the file names.
  $project_name = _kraftwagen_drush_prompt_validate(
    dt('Enter the internal machine name of the project'), 
    _kraftwagen_generate_name($project_label), TRUE, '_kraftwagen_validate_name'
  );
  
  // ask for the location of the git repository that contains the skeleton.
  $skeleton = drush_prompt(dt('Enter the location of the skeleton repository'));
  $branch = drush_prompt(dt('Enther the name of the branch to use [source repository HEAD]'), NULL, FALSE);

  // create a tempdir where we are going to clone the repository in
  $tempdir = drush_tempdir();
  
  // clone the skeleton repository
  if ($branch) {
    $args = array('git clone -b %s %s %s', $branch, $skeleton, $tempdir);
  } else {
    $args = array('git clone %s %s', $skeleton, $tempdir);
  }
  if (call_user_func_array('drush_shell_exec', $args)) {
    // remove the repository nature of the thing
    if (file_exists($tempdir . '/.git')) {
      drush_shell_exec("rm -rf %s", $tempdir . '/.git');
    }

    // make sure we have a src directory. This is going to contain the whole 
    // actual source of the project.
    drush_mkdir($dir . DIRECTORY_SEPARATOR . $dir_src);
    
    kraftwagen_copy_dir($tempdir, $dir . DIRECTORY_SEPARATOR . $dir_src, 
      array('SKELETON' => $project_name),
      array(
        '***MACHINE_NAME***' => $project_name, 
        '***HUMAN_NAME***' => $project_label, 
        '***HUMAN_NAME_ESCAPED***' => addslashes($project_label)
      )
    );

    drush_log(dt('Copied the project skeleton to your source directory.'), 'ok');
  } else {
    return drush_set_error(dt('Could not download the repository from %path', array('%path' => $skeleton)));
  }
}

/**
 * Create a system name from a human readable name
 */
function _kraftwagen_generate_name($label) {
  // convert to lowercase, replace every non-alphanumeric character with an 
  // underscore, replace all repeating underscores with a single underscore, 
  // make sure that the name does not start with a number
  return preg_replace('/^([0-9])/','_\1', preg_replace('/_+/','_', preg_replace('/[^a-z0-9]/', '_', strtolower($label))));
}

/**
 * Check if a given name is a valid system name
 */
function _kraftwagen_validate_name($name) {
  // make sure it consists only of alphanumeric characters and underscores and
  // does not start with a number. 
  $valid = preg_match('/^[a-z_][a-z0-9_]*$/', $name);
  if (!$valid) {
    drush_set_error(dt('Only lowercase alphanumeric characters and underscores are allowed. The input cannot start with a number.'));
  }
  return $valid;
}

/**
 * Create drush prompt that runs the input through a validate callback
 */
function _kraftwagen_drush_prompt_validate($prompt, $default = NULL, $required = TRUE, $validate_callback = NULL) {
  while (TRUE) {
    $result = drush_prompt($prompt, $default, $required);
    
    // assume that the default is a valid result
    if ($result == $default) {
      return $result;
    }

    // if we have a callback and the callback returns true
    if ($validate_callback && $validate_callback($result)) {
      return $result;
    }
  }
}


function _kraftwagen_copy_dir($src_dir, $dest_dir, $filename_replacements = array(), $content_replacements = array()) {
  // find the files in the src_dir. The grep and sed, remove 
  // that prefixing dot
  drush_shell_cd_and_exec($src_dir, 'find . -mindepth 1 | grep "^\." | sed -e "s/^\.//"');
  $files = drush_shell_exec_output();

  // make sure the kraftwagen_file_replace function is available
  require_once dirname(__FILE__) . '/kraftwagen.fileutils.inc';
  
  // loop over the files. $file starts with a slash
  foreach ($files as $file) {
    // create the full path of the file in the src_dir
    $src_path = $src_dir . $file;
    // create the full path that the file should get in the working directory
    $dest_path = $dest_dir . (!empty($filename_replacements) ? str_replace(array_keys($filename_replacements), array_values($filename_replacements), $file) : $file);
    
    if (is_dir($src_path)) {
      drush_mkdir($dest_path);
    } else {
      kraftwagen_file_replace($src_path, $dest_path, $content_replacements);
    }
  }
}
