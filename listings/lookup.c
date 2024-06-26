void Lookup(meta, key, hash, result)
{
  // assumptions: - meta, key, result != NULL

  d_0 = Distance0Of(hash, meta);

beginning:
  is_compact = meta->is_compact;
  non_compacts = 0;

  position;

  for (d = 0; d < meta->size; d++) {
    position = (d_0 + d + non_compacts) & (meta->size - 1);
    node = meta->index[position];

    if (IsNonCompact(node, meta)) {
      // note: tombstones are non-compacts to entry #0

      d--;
      non_compacts++;
      result->is_non_compact = true;
      goto check_entry;
    }
    result->is_non_compact = false;

    if (node == EMPTY) {
      goto not_found;
    }

    if (is_compact && (
      (d_0 + d + non_compacts - node.d > d_0)
      || (d >= meta->max_d)
    )) {
      goto not_found;
    }

check_entry:
    if (node.tag == hash) {
      result->entry_p = GetEntryAt(node.location, meta);
      ReadEntry(result->entry_p. &result->entry);

      if (result->entry.value == NULL) {
        continue;
      }

      if (result->entry.key == key) {  // identity
        goto found;
      }

      if (result->entry.hash != hash) {
        continue;
      }

      cmp = PyObject_Equals(result->entry.key, key);

      if (cmp < 0) {  // exception thrown during compare
        goto error;
      }

      if (cmp == 0) {  // == false
        continue;
      }
      goto found;  // => true
    }
  }  // probes exhausted

not_found:
  if (is_compact != meta->is_compact) {
    goto beginning;
  }
  result->error = false;
  result->found = false;
  return;
found:
  result->error = false;
  result->found = true;
  result->position = position;
  result->node = node;
  return;
error:
  result->error = true;
}
