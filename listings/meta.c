Meta *getMeta(dict)
{
  do {
    ref = dict->meta;
  } while (!TryIncref(ref));

  return ref;
}

void releaseMeta(meta)
{
  Decref(meta);
}
