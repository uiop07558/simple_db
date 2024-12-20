# Simple DB
_Из-за технических неполадок, на данный момент работа завершена не полностью. На данный момент готова основная часть БД - движок, представляющий собой БД типа "ключ/значение", функционал; требуемый в лабораторной должен быть реализован на основе движка_

### Производительность
Движок сохраняет данные в структуре данных B+tree, что позволяет добиться логарифмической сложности операций чтения, записи, удаления

##### Точнее:
- **n** - кол-во записей в таблице (дереве)
- **k** - max кол-во элементов в узле дерева (странице)
- **k <= 371 = (PAGE_SIZE - sizeof(InternalHeader)) / (sizeof(InternalSlot) + 1)** (макс кол-во элементов во внутреннем узле, если все ключи занимают 1 байт)

- Кол-во дисковых операций - O(log_k(n)) = O(log(n))
- Кол-во операций с ОЗУ (внутри страницы) при чтении - O(log(k)) = O(1)
- Кол-во операций с ОЗУ (внутри страницы) при записи и удалении - O(k) = O(1) (т.к k ограничено)

### Работа с памятью
Движок работает со страницами размера 4 КБ, процесса сериализации/десериализации не происходит, в памяти страницы представлены для чтения и манипуляции в том же бинарном формате, что и на диске. Ввод/вывод проходит через механизм mmap, позволяющий работать с файлом БД, как с областью памяти процесса. Помимо этого движок поддерживает транзакции - изменения внутри транзакции сохраняются в памяти, и только при вызове commit записываются на диск.
