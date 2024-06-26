PyObject *ExpectedInsertOrUpdate(meta, key, hash, expected,
                                 desired, entry_loc, *must_grow)
{
  // assumptions:
  //   - key \not\in {NOT_FOUND, ANY, EXPECTATION_FAILED}
  //   - expected \not\in {EXPECTATION_FAILED}
  //   - desired \not\in {NOT_FOUND, ANY, EXPECTATION_FAILED}
  //   - (expected \in {NOT_FOUND, ANY}) => entry_loc != NULL
  //   - entry_loc != NULL => (
  //       entry_loc->key == key
  //       && entry_loc->value == desired
  //       && entry_loc->hash == hash
  //     )
  //   - meta, key, expected, desired != NULL

  d_0 = Distance0Of(hash, meta)
  node = EMPTY

  if (expected == NOT_FOUND || expected == ANY) {
    // insert quick path
    node.location = entry_loc->location;
    node.distance = 0;
    node.tag = hash;

    if (CompareAndSet(meta->index[d_0], EMPTY, node)) {
      return NOT_FOUND;  // success
    }
  }

beginning:
  done = false; expectation = true; d = 0; current = NULL;
  is_compact = meta->is_compact;
  to_insert = EMPTY;

  if (ExpectedInsertOrUpdateCloseToDistance0(/* ... */) < 0) {
    goto fail;
  }

  // the above call sets done = true on success

  while (!done) {
    pos = (d_0 + d) & (meta->size - 1);
    node = meta->index[pos]

    if (node == EMPTY) {
      if (expected != NOT_FOUND && expected != ANY) {
        done = true; expectation = false;
        break;
      }
      else {
        if (is_compact) {
          meta->is_compact = false;
          is_compact = false;
        }

        done = CompareAndSet(meta->index[pos], EMPTY, NewNode(
          entry_loc->location, meta->max_d, hash
        ));

        if (!done) {
          continue;  // retry at the same position
        }
      }
    }
    else if (node == TOMBSTONE) {
      // do nothing (i.e. d++)
    }
    else if (is_compact && (
      !IsNonCompact(node, meta) && (pos - node.d > d_0)
    )) {
      if (expected != NOT_FOUND && expected != ANY) {
        done = true; expectation = false;
        break;
      }
    }
    else if (node.tag != hash) {
      // skip
    }
    else {
      updated = ExpectedUpdateEntry(/* ... */);

      if (updated < 0) {
        goto fail;
      }

      if (updated) {
        break;
      }
    }

    d++;

    if (d >= meta->size) {
      // traversed the entire dictionary
      // without finding an empty slot
      *must_grow = true;
      goto fail;
    }
  }

  // note: expected == ANY => expectation == false

  if (expected != NOT_FOUND && expectation == false
    && meta->is_compact != is_compact
  ) {
    goto beginning;
  }

  if (expectation == true && expected == NOT_FOUND) {
    return NOT_FOUND;
  }
  else if (expectation == true && expected == ANY) {
    if (current == NULL) {
      return NOT_FOUND;
    }
    return current;
  }
  else if (expectation == true) {
    return current;
  }
  else {
    return EXPECTATION_FAILED;
  }

fail:
  return NULL;
}
