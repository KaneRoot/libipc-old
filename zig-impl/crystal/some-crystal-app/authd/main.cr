require "uuid"
require "option_parser"
require "openssl"
require "colorize"
require "jwt"
require "grok"

require "dodb"
require "baguette-crystal-base"

require "../src/main.cr"
require "./libauth.cr"
require "./authd.cr"
