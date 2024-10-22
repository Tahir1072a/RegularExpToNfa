# RegularExpToNfa
> **Description:** The C code that obtains an NFA machine from a regular expression using the Thompson construction algorithm has been shared. 

Bu kod +,* operatorlerine sahip bir reguler expression ifadesini, nfa diyagramına 
dönüştürür. Ayrıca bu verilen kelimelerin bu dile ait olup olmadığını veya bu kurala 
göre kelime üretimini sağlar.

> **Note:**  '+' veya operatörü olarak kullanılmıştır. Bazı kaynaklarda '|' olarak da
> kullanılır.

### Postfix To Nfa Algorithm
Bu algoritmanın arkasında yatan mantık, postfix ifadenin en iç kısmından başlayarak
buna göre kısmi nfa'lar oluşturmaktır. Nfa bu kısmi nfa'ların birleştirilmesinden
meydana gelir. Biz her bir kısmi nfa'yı fragment olarak ifade ederiz. 

Fragment içinde bir state dizisi ve bu fragmenta ait birleştirilmemiş yolları/oklarını tutar.
Bu fragmnetlar regular expr. göre adım adım birleştirilerek son nfa parçasını oluşturur.
En son bu nfa'nın boşta kalan oklarını match(bitiş) state'ine bağlayarak nfa diyagramı
tamamlanmış olur.