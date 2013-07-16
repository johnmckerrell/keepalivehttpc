Simple HTTP Client
------------------

 * John McKerrell (c) 2013
 * Released under the Perl Foundation Artistic License 2013
  * http://www.perlfoundation.org/artistic_license_2_0

A very simple HTTP client, accepts a URL on the command line and outputs the
response headers and body. Also accepts -I argument for outputting the request
headers.

Key feature is that it sets the SO_KEEPALIVE socket option on so that TCP/IP
keepalive packets will be set. This is aimed at supporting long polling.
Check the default keepalive settings in your OS as you may need to change
them for this to be useful. There is commented out code that will do this
within the app but this didn't work on the target OS (OpenWRT) so was disabled.

Future improvements would be adding the ability to post data taken from
standard input.

