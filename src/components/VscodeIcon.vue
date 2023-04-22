<template>
  <div :class="cls" style="font-size:18px"></div>
</template>

<script setup lang="ts">
const {name} = defineProps<{
  name: string,
}>()

const cls = ['codicon', `codicon-${name}`]
</script>