.pragma library

function merge(old, newOne) {
  if (!newOne)
      return old

  for (const field in newOne) {
    old[field] = newOne[field]
  }
  return old
}
