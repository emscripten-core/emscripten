from collections import namedtuple
try:
  from StringIO import StringIO
except ImportError:
  from io import StringIO


class SettingsParser:
  make_token = namedtuple('token', 'kind value')

  def __init__(self, stream):
    if type(stream) is str:
      stream = StringIO(stream)
    self.stream = stream
    self.token_buffer = []
    self.char_buffer = []

  def get_token(self):
    if self.token_buffer:
      return self.token_buffer.pop()

    def end():
      return peek() == ''

    def peek():
      if self.char_buffer:
        return self.char_buffer[-1]
      ch = self.stream.read(1)
      self.char_buffer.append(ch)
      return ch

    def pop():
      if self.char_buffer:
        return self.char_buffer.pop()
      return self.stream.read(1)

    def pop_while(chars):
      s = ''
      while not end() and peek() in chars:
        s += pop()
      return s

    def pop_until(chars):
      s = ''
      while not end() and peek() not in chars:
        s += pop()
      return s

    space_chars = ' '
    op_chars = '[]=,;@'
    non_word_chars = op_chars + space_chars + '#\n'

    pop_while(space_chars)
    if end():
      token = self.make_token('endmarker', '')
    elif peek() in op_chars:
      token = self.make_token('op', pop())
    elif peek() == '#':
      token = self.make_token('comment', pop_until('\n'))
    elif peek() == '\n':
      token = self.make_token('newline', pop())
    else:
      res = ''
      while not end() and peek() not in non_word_chars:
        if peek() in '"\'':
          qch = pop()
          res += pop_until(qch)
          if end():
            raise Exception('unclosed opened quoted string. expected final character to be "%s"' % qch)
          pop()
        else:
          res += pop()
      token = self.make_token('string', res)
    if token.kind == 'comment':
      return self.get_token()
    return token

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
