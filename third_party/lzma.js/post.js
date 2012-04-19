
    return returnValue;
  },

  compress: function(data) {
    return this.run(data);
  },

  decompress: function(data) {
    return this.run(data, true);
  }
};

