<template>
  <ElButton text class="toolButton">
    <ElIcon size="24"><slot/></ElIcon>
    {{label}}
  </ElButton>
</template>

<script lang="ts" setup>

const {label} = defineProps<{
  label: string,
}>()

</script>

<style>

.toolButton {
  height: 100% !important;
  padding: 5px 4px !important;
}
.toolButton>span {
  display: flex;
  flex-direction: column;
  height: 100%;
  justify-content: space-around;
  font-size: 14px;
}

</style>