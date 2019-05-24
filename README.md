# configer

一个贼简单的在php用于解析toml配置文件的小扩展，用于学习php姿势

## Installation

跟大多数扩展装法一样

## Usage

### INI configuration entries

```ini
configer.conffile_path=/path/to/that.toml
configer.namespace=somenamespace
```

### Use in PHP code

/path/to/that.toml:
```toml
[a]
b="c"
```

```php
<?php
var_dump(\somenamesapce\getConfig());
```

## license
MIT

## acknowledgement

This project used cktan's [tomlc99](https://github.com/cktan/tomlc99) as it's parser.
