from collections import namedtuple
from tools.shared import StringScanner


class SettingsParser:
  def __init__(self, s):
    self.s = StringScanner(s)
    self.token_buffer = []

  def get_token(self):
    if self.token_buffer:
      return self.token_buffer.pop()

    s = self.s

    # skip spaces
    s.scan(' *')

    if s.eos():
      kind = 'endmarker'
      value = ''
    elif s.peek() in '[]=,;@':
      kind = 'op'
      value = s.get_char()
    elif s.peek() == '#':
      kind = 'comment'
      value = s.scan('[^\n]+')
    elif s.peek() == '\n':
      kind = 'newline'
      value = s.get_char()
    else:
      kind = 'string'
      value = ''
      non_word_chars = '[]=,;@ #\n'
      while not s.eos() and s.peek() not in non_word_chars:
        if s.peek() in '"\'':
          qch = s.get_char()
          value += s.scan('[^%s]+' % qch)
          if s.eos():
            raise Exception('unclosed opened quoted string. expected final character to be "%s"' % qch)
          s.get_char()
        else:
          value += s.get_char()

    # skip comments
    if kind == 'comment':
      return self.get_token()

    return namedtuple('Token', 'kind value')(kind, value)

  def peek_token(self):
    if self.token_buffer:
      return self.token_buffer[-1]
    token = self.get_token()
    self.token_buffer.append(token)
    return token

  def parse_string(self):
    token = self.peek_token()
    if token.kind != 'string':
      raise Exception('expected string, got "%s"', token.value)
    return self.get_token().value

  def parse_list(self):
    brackets = False
    if self.peek_token() == ('op', '['):
      brackets = True
      self.get_token()

    res = []
    while 1:
      token = self.peek_token()
      if token == ('op', ',') or (brackets and token.kind == 'newline'):
        self.get_token()
        continue
      if token.kind == 'string':
        res.append(self.parse_string())
      else:
        break

    if brackets:
      if self.peek_token() != ('op', ']'):
        raise Exception('unclosed opened string list. expected final character to be "]"')
      self.get_token()

    return res

  def parse_value(self):
    if self.peek_token() == ('op', '@'):
      # response file
      self.get_token()
      return '@' + self.parse_string()
    else:
      return self.parse_list()

  def parse_setting(self):
    key = self.parse_string()
    token = self.peek_token()
    if token != ('op', '='):
      raise Exception('expected "=" after variable name, got "%s"' % token.value)
    self.get_token()
    value = self.parse_value()
    token = self.peek_token()
    if token.kind not in ['endmarker', 'newline'] and token != ('op', ';'):
      raise Exception('expected newline or ";", got "%s"' % token.value)
    self.get_token()
    return (key, value)

  def parse_settings(self):
    settings = {}
    while self.peek_token().kind != 'endmarker':
      token = self.peek_token()
      if token == ('op', ';') or token.kind == 'newline':
        self.get_token()
        continue
      if token.kind == 'string':
        key, value = self.parse_setting()
        settings[key] = value
      else:
        raise Exception('expected variable name, got "%s"', token.value)
    return settings
