int Delete(meta, key, hash)
{
  // assumptions:
  //   - meta, key != NULL
  //   - key \not\in {NOT_FOUND, ANY, EXPECTATION_FAILED}

  result = Lookup(meta, key, hash)

  if (result.error) {
    goto fail;
  }

  if (!result.found) {
    goto not_found;
  }

  while (!CompareAndSet(&result.entry_p->value, result.entry.value, NULL)) {
    ReadEntry(result.entry_p, &result.entry);

    if (result.entry.value == NULL) {
      goto not_found;
    }
  }

  result.entry_p->key = NULL;
  Decref(result.entry.key);
  Decref(result.entry.value);
  result.entry.value = NULL;

  should_shrink = false;

  page = PageOf(result.entry_p);
  if (AddThenFetch(page->del_counter, 1) >= PAGE_SIZE / 2) {
    BeginSyncOp();

    p_0, p_1, p_2 = NULL;
    p = 0;

    for (; p <= meta->greatest_allocated_page; p++) {
      if (Size(p) > 0) {
        p_0 = p;
        break;
      }
    }

    for (; p <= meta->greatest_allocated_page; p++) {
      if (p->del_counter >= PAGE_SIZE / 2 && p != page) {
        p_1 = p;
        break;
      }
    }

    if (p_0 != NULL && p_1 != NULL) {
      if (page > p_1) {
        p_2 = page;
      }
      else {
        p_2 = p_1;
        p_1 = page;
      }

      Merge(p_0, p_1, p_2);  // cannot fail

      if (Size(p_0) == 0) {
        Decref(meta->pages[p_0]);
        meta->pages[p_0] = NULL;

        meta->greatest_deleted_page++;
        used_pages = meta->greatest_allocated_page
          - meta->greatest_deleted_page
          + meta->greatest_refilled_page;
        if ((used_pages) * PAGE_SIZE <= meta->size * MIN_USED_RATIO) {
          should_shrink = true;
        }
      }
    }

    EndSyncOp();
  }

  return 1;

not_found:
  return 0;

fail:
  return -1;
}
