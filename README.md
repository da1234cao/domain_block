

```shell
sc.exe create domain_block_service binpath=E:\code\self\domain_block\windows\WFP\bin\Debug\domain_block_service.exe type=own start=demand error=normal

 sc start domain_block_service
```