# Features Revert Project (Drush command)

Did you have a situation when your Drupal project contains a lot of features
that need to be reverted after the regular code deployment? The `drush frp`
command is enabled/reverted the necessary modules/features.

To let the command know what should be checked, a special file in "sites/default"
that named "components.info" should be created. Also, the contents of that file
can be placed into an installation profile "*.info" file and described previous 
step can be ommited.

Example of contents from "*.info" file:
```
; Features
components[] = feature_name

; Modules
components[] = module_name
```

## Usage

Enable or revert all modules or features if it necessary.
```
drush frp
```

## Installation

You can just execute the `drush dl features_revert_projects` and Drush download
the package into your `~/.drush` folder.

Another way to install this command - is manually download the sources and
place it into `~/.drush` folder by yourself.
