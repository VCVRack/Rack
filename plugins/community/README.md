# VCV community repository

The VCV community members are responsible for curating Rack plugins into the [VCV Plugin Manager](https://vcvrack.com/plugins.html).
[Anyone is welcome to join.](https://github.com/VCVRack/community/issues/248)

All Rack plugins are welcome assuming they
- are not malware (i.e. harm your computer or your privacy)
- do not misuse intellectual property (legally or morally)


# Adding your plugin to the VCV Plugin Manager

Create exactly one thread in the [Issue Tracker](https://github.com/VCVRack/community/issues), with a title equal to your plugin slug (or multiple slugs, comma-separated, if you have more than one plugin).
This will be your permanent communication channel with VCV community members.


#### Adding/updating your plugin's information

Post a comment in your plugin's thread with the plugin name, license, all relevant URLs, and your email address if you want it to be public.

A Library team member will handle your request and post a comment when updated.


#### Adding/updating your plugin's build (for open-source plugins)

To inform us of an update to the plugin itself, make sure to increment the `VERSION` in your Makefile (e.g. from 0.6.12 to 0.6.13), and push a commit to your repository.
Post a comment in your plugin's thread with
- the new version (e.g. `0.6.42`)
- the commit hash (given by `git log` or `git rev-parse HEAD`. Please do not just give the name of a branch like `master`.)

A Review team member will handle your request and post a comment when updated.


#### Adding/updating your plugin's build (for closed-source free and commercial plugins)

Email contact@vcvrack.com to be added to the VCV Plugin Manager or sold through the VCV Store.
It is not necessary to have a plugin thread, although you may create one if you like.


## Manifest files

The path of each manifest should be `manifests/YourSlug.json`.
See [manifest/Fundamental.json](manifests/Fundamental.json) for an example.

All properties are currently optional, but it is recommended to enter as much information as possible. URLs should not be redundant across different keys, e.g. you should not add a `pluginUrl` if it is the same URL as `sourceUrl`.

- **name**: Human-readable display name for your plugin. You can change this on a whim, unlike slugs.
- **author**: Your name, company, alias, or GitHub username.
- **license**: The license type of your plugin. Use "proprietary" if all rights are reserved. If your license is in the [SPDX license list](https://spdx.org/licenses/), use its abbreviation in the "Identifier" column.
- **authorEmail**: Your email address for support inquiries.
- **pluginUrl**: Homepage featuring the plugin itself.
- **authorUrl**: Homepage of the author.
- **manualUrl**: The manual of your plugin. HTML, PDF, or GitHub readme/wiki are fine.
- **sourceUrl**: The source code homepage. E.g. GitHub repo.
- **donateUrl**: Link to donation page for users who wish to donate. E.g. PayPal URL.
- **latestVersion**: Your plugin's latest version, using the guidelines at https://github.com/VCVRack/Rack/issues/266. Do not include the "v" prefix.
- **productId**: ID for plugins sold through the VCV Store.
- **status**: *TODO*


## Building repos

Clone all repos with `git submodule update --init --recursive`

Then build all repos with `RACK_DIR=<path to Rack directory> make -j$(nproc) dist_all`

## Adding a repo

The folder name should match the slug, even if it is not the repository name.
Be sure to check out the correct branch.

```
cd repos
git submodule add -b master https://github.com/VCVRack/Fundamental.git Fundamental
```
