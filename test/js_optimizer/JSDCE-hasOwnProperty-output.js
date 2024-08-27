function hasOwnProperty(obj, prop) {
  return Object.prototype.hasOwnProperty.call(obj, prop);
}

if (hasOwnProperty({}, "prop_name")) {
  console.log("yeah");
}
