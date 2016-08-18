# yubimon

yubimon is a set of tools for working with the
[Yubikey4](https://www.yubico.com/products/yubikey-hardware/yubikey4/) USB
fob on Mac OS X.  Specifically, it will prompt the user for their
Yubikey PIN upon insertion of the fob, and add the hardware SSH key
to their SSH keyring.  It reverses this process when the fob is
removed.

## Dependencies

yubimon relies on a few other tools to do its job.  If
[Homebrew](http://brew.sh) is available, yubimon can walk the user
through the installation of these dependencies.

## Installation

```
bash$ make install
```

## Usage

yubimon installs as a `launchd` managed daemon.  No muss, no fuss.
It will log messages to `/var/log/system.log`.  Look there if things
seem broken.

## Contributing

1. Fork it!
2. Create your feature branch: `git checkout -b my-new-feature`
3. Commit your changes: `git commit -am 'Add some feature'`
4. Push to the branch: `git push origin my-new-feature`
5. Submit a pull request :D

## License

The MIT License (MIT)
Copyright (c) 2016 Cyrus Durgin

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject
to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

