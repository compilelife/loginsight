<template>
  <div class="toolArea">
    <slot name="up"/>
    <slot name="down" />
  </div>
</template>

<style>
.toolArea {
  display: flex;
  flex-direction: column;
  align-items: flex-start;
  row-gap: 3px;
}

.toolArea span {
  white-space: nowrap;
}
</style>