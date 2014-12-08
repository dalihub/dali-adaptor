Dali Browser Regression Tests
-----------------------------

These tests work with a web server but not from the filesystem (a "file:///" URI).

To run the tests locally run a local web server.

One quick way to do this if you have nodeJS and npm installed is

npm install http-server -g

then in the directory with dali_tests.html run

http-server

or

http-server -p:8888

if you already have a local server running on 8080.

Note: Alternatively you can use python with

python -m SimpleHTTPServer 8080

