var customMessageData = '';

Module.onCustomMessage = function (message) {
  var data = message.data.userData;
  switch (data.op) {
    case 'preMainCustomMessage': {
      customMessageData += data.data;
      break;
    }
    case 'runMain': {
      postCustomMessage({ op: 'runningMain', data: customMessageData });
      removeRunDependency('Custom Message Init');
      break;
    }
    case 'postMainCustomMessage': {
      customMessageData += data.data;
      break;
    }
    case 'finish': {
      postCustomMessage({ op: 'finishing', data: customMessageData + '[finish]' });
      break;
    }
    default: {
        throw 'unknown custom message';
    }
  }

}

addRunDependency('Custom Message Init');
