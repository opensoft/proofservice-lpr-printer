Upgrades
========
Changes that one must make to applications based on Proof between versions.

## Not Released
#### IT issues
*Nothing to do*

#### Config changes
*Config was not changed*

#### API modifications/removals/deprecations
 * ObjectsCache::add() now accepts one more optional argument *overwriteCurrent* and returns shared pointer either to inserted object or (if *overwriteCurrent==true* and cache already contains this key) existing value
