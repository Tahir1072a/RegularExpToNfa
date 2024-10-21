#include <stdio.h>
#include <stdlib.h>
#include <string.h> //  strlen fonskiyonu kullanıldı.
#include <time.h> //    Rastgele sayı üretmek için kullanıldı
#include <math.h> //    sadece pow fonskiyonu kullanıldı

#define Split 256
#define Match 257

typedef struct State State;
typedef struct Fragmentation Frag;
typedef struct PointerList ptr_list;
typedef struct StateList List;

struct State {
    int c;
    State *out;
    State *out1;
    int lastlist;
};

struct Fragmentation {
    State *start;
    ptr_list *outs;
};

struct PointerList {
    State **out;
    ptr_list *next;
};

struct StateList {
    State **states;
    int count;
};

//Global variables...
State matchState = {Match, NULL, NULL};
List l1 = {NULL, 0};
List l2 = {NULL, 0};
int listid = 0;

// Building NFA
State* createState(int c, State *out, State *out1);
Frag createFrag(State* start, ptr_list *outs);
ptr_list* createList(State** out);
ptr_list* append(ptr_list* l1, ptr_list* l2);
void patch(ptr_list* l1, State* s);
State* nfaBuilder(char* postfix);

// Word Controls
int checkWord(State* start, char* word);
int isMatch(List *l);
void addState(List *l, State *s, int* listid);
List* startList(State *s, List* l, int* listid);
void Step(List* clist, int c, List* nlist);

//Convert infix to postfix
char* convertToPostfix(char* infix);

// Word production
void wordProduction(char* alphabet, int numberOfWords, State* start);
char* separateAlphabet(char* alphabet);

int main() {
    srand(time(NULL));
    char p[1000], alphabet[40];
    int control = 1, numberOfWords = 0;

    l1.states = malloc(sizeof(State*));
    l2.states = malloc(sizeof(State*));

    printf("Lütfen üzerinde çalışılacak alfabeyi giriniz: Not: alfabeyi a,b,c,d şeklinde girin.\n");
    scanf("%s", alphabet);
    printf("Lütfen işlenecek infix ifadeyi giriniz: \n");
    scanf("%s", p);
    char* postfix = convertToPostfix(p);
    printf("İşlenecek olan postfix ifade: %s\n", postfix);
    State *start = nfaBuilder(postfix);

    printf("Ne işlemi yapmak istiyorsunuz?\n 1-Verdiğim kurala göre belli sayıda kelime üretmek istiyorum\n 2-Verdiğim kelimenin bu dile ait olup olmadığını öğrenmek istiyorum.\n");
    printf("Lütfen 1 yada 2 numaralı tuşa basınız...\n");
    scanf("%d", &control);

    switch (control) {
        case 1:
            printf("Dile ait kaç kelime görmek istiyorsunuz?");
            scanf("%d", &numberOfWords);
            wordProduction(alphabet, numberOfWords, start);
            break;
        case 2:
            while (control) {
                printf("Lütfen kontrol etmek istediğiniz kelimeyi giriniz:\n");
                scanf("%s",p);
                int result = checkWord(start, p);

                if (result == 1) printf("Evet bu kelime ilgili dile ait bir kelimedir: %s\n",p);
                else printf("Bu kelime ilgili dile ait değildir! %s\n",p);
                printf("Çıkmak için 0'e basiniz:\n");
                scanf("%d", &control);
            }
            break;
        default:
            printf("Yanlış bir tuşlama yaptınız. Doğru tuşlama yapmayı öğrenene kadar lütfen programı kullanmayın!");
            break;
    }

    return 0;
}

ptr_list* createList(State** out) {
    ptr_list* list = (ptr_list*)malloc(sizeof(ptr_list));

    list->out = out;
    list->next = NULL;

    return list;
}

ptr_list* append(ptr_list* l1, ptr_list* l2) {
    ptr_list* head;

    if(l1 == NULL) return head = l2;
    head = l1;
    while (head->next != NULL) head = head->next;
    head->next = l2;

    return l1;
}

// Parametre olarak aldığı pointer listesini ki bu boşta kalan okları temsil eder, bunları belirtilen state'e bağlar.
void patch(ptr_list* l1, State* s) {
    if(l1 == NULL) return; //Liste boş olması durumda bağlanacak ok yoktur.
    ptr_list *next;
    while (l1 != NULL) {
        *(l1->out) = s; //Artık ilgili ok bir adresi gösteriyor. Önceden null bir değeri gösteriyordu.
        l1 = l1->next;
    }

    free(l1);
}

State* createState(int c, State* out, State* out1) {
    State* s = (State*)malloc(sizeof(State));
    s->c = c;
    s->out = out;
    s->out1 = out1;

    return s;
}

Frag createFrag(State* start, ptr_list* outs) {
    Frag fragment = {start, outs};
    return fragment;
}

// Epsilon yollarıda içeren bir nfa diyagramı oluşturur.
State* nfaBuilder(char* postfix) {
    char *p; // Pointer for the postfix exp.
    // Kullanılacak veri yapıları
    Frag stack[1000], *stackp, e1, e2;
    State* s;
    //Stack func.
    #define Push(s) *stackp++ = s;
    #define Pop() *--stackp;

    stackp = stack;
    for (p = postfix; *p; p++) {
        switch (*p) {
            case '+':
                // e1 ve e2 birer fragmenttır. + işareti veya anlamı katar.
                // Yani fragmentlerdan herhangi birisine geçiş yapabiliriz. Bunu ifade etmek için de split durumunda bir state oluşturalım.
                e2 = Pop(); //İlk çıkanın e2 ye atanması postfix ifadenin okunması durumda oluşan sıradan kaynaklanmaktadır.
                e1 = Pop();
                s = createState(Split, e1.start, e2.start); //Split durumda state oluşturuldu.
                Push(createFrag(s, append(e1.outs, e2.outs))); // Fragment oluşturulur ve state içine itilir.
                //append fonksiyonu iki fragment çıkış oklarını tek bir liste olarak geri döner.
            break;
            case '*':
                e1 = Pop();
                s = createState(Split, e1.start, NULL);
                patch(e1.outs, s);
                Push(createFrag(s, createList(&s->out1)));
            break;
            case '.':
                // Yeni bir state ihtiyaç yoktur. İlgili fragmentları sıraya göre birbirine bağlar. Bağlama işlemi boşta olan oklar ile yapılır.
                e2 = Pop();
                e1 = Pop();
                patch(e1.outs, e2.start);
                Push(createFrag(e1.start, e2.outs));
                break;
            default:
                s = createState(*p, NULL, NULL);
                Push(createFrag(s, createList(&s->out)));
                break;
        }
    }
    // Buradaki işlem artık stack içinde tek bir fragment kalmış ise ilgili fragment'ın sallanan oklarının herhangi bir state bağlanma zorunluluğu
    // olmadığını gösterir. Bunun anlamı ise bu oklar final state bağlanması gerekir.
    e1 = Pop();
    patch(e1.outs, &matchState); //Okları önceden tanımlanmış matchState'e bağlar.
      return e1.start;
}

int checkWord(State* start, char* word) {
    if (word == NULL) return 0;

    List *clist, *nlist, *t;
    clist = startList(start, &l1, &listid);
    nlist = &l2;
    for(; *word; word++) {
        Step(clist, *word, nlist);
        t = clist;
        clist = nlist;
        nlist = t;
    }
    return isMatch(clist);
}
int isMatch(List *l) {
    for (int i = 0; l->count > i; i++) {
        if(l->states[i] == &matchState) return 1;
    }
    return 0;
}

void addState(List *l, State *s, int* listid) {
    if(s == NULL || s->lastlist == *listid) return;
    s->lastlist = *listid;
    if(s->c == Split) {
        addState(l, s->out,listid);
        addState(l, s->out1, listid);
        return;
    }
    l->states[l->count++] = s;
}

List* startList(State *s, List* l, int* listid) {
    (*listid)++;
    l->count = 0;
    addState(l,s, listid);
    return l;
}

void Step(List* clist, int c, List* nlist) {
    State *s;
    listid++;
    nlist->count = 0;
    for(int i = 0; i < clist->count; i++) {
        s = clist->states[i];
        if(s->c == c)
            addState(nlist, s->out, &listid);
    }
}

char* convertToPostfix(char* infix) {
    char stack[1000];
    int stack_counter = -1;
    int letter_counter = 0;

    char *postfix = (char*)malloc(1000 * sizeof(char));
    char *ptr = postfix;

    for (;*infix; infix++) {
        char temp = *infix;
        switch (temp) {
            case '+':
                while (stack_counter != -1 && stack[stack_counter] ==  '.' || stack[stack_counter] == '*' || stack_counter == '+') {
                    *ptr++ = stack[stack_counter--];
                }
            stack[++stack_counter] = temp;
            letter_counter = 0;
            break;
            case '*':
                while (stack_counter != -1 && stack[stack_counter] == '*'){
                    *ptr++ = stack[stack_counter--];
                }
            stack[++stack_counter] = temp;
            break;
            case ')':
                while (stack[stack_counter] != '(') *ptr++ = stack[stack_counter--];
            stack_counter--;
            letter_counter++;
            break;
            case '(':
                if (letter_counter >= 1) {
                    while (stack_counter != -1 && (stack[stack_counter] == '.' || stack[stack_counter] == '*'))
                        *ptr++ = stack[stack_counter--];
                    stack[++stack_counter] = '.';
                    letter_counter = 0;
                }
                stack[++stack_counter] = '(';
                break;
            default:
                letter_counter++;
                if(letter_counter >= 2) {
                    while (stack_counter != -1 && (stack[stack_counter] == '.' || stack[stack_counter] == '*'))
                        *ptr++ = stack[stack_counter--];
                    stack[++stack_counter] = '.';
                    letter_counter--;
                }
                *ptr++ = temp;
                break;
        }
    }

    if (stack_counter != -1) {
        for (; stack_counter > -1; stack_counter--) {
            *ptr++ = stack[stack_counter];
        }
    }
    *ptr = '\0';
    return postfix;
}

char* separateAlphabet(char* alphabet) {
    char* setOfAlphabet = malloc(40 * sizeof(char));

    for (int i = 0; strlen(alphabet) > i; i++) {
        if (alphabet[i] == ',') continue;
        char temp[2];
        temp[0] = alphabet[i];
        temp[1] = '\0';
        strcat(setOfAlphabet, temp);
    }

    return setOfAlphabet;
}

int isWordInArray(char** words, int count, const char* word) {
    for (int i = 0; i < count; i++) {
        if (strcmp(words[i], word) == 0) {
            return 1; // Kelime bulundu
        }
    }
    return 0; // Kelime bulunamadı
}

void wordProduction(char* alphabet, int numberOfWords, State* start) {
    char* setOfAlphabet = separateAlphabet(alphabet);
    char  word[100] = "";
    char** words = (char**)malloc((numberOfWords + 1) * sizeof(char*));
    char** wordsp = words;

    int generatedCount = 0;

    for (int i = 1; numberOfWords > 0; i++) {
        int calculated = (int)pow((int)strlen(setOfAlphabet), i) * i;
        for (int j = 0; calculated > j; j++) {
            if (numberOfWords <= 0) break;

            char temp[2];
            temp[0] = setOfAlphabet[rand() % (int)strlen(setOfAlphabet)];
            temp[1] = '\0';

            strcat(word, temp);

            if (i == strlen(word)) {
                if (checkWord(start, word) && !isWordInArray(words, generatedCount, word)) {
                    for (int k = 0; k < strlen(words);k++) {

                    }
                    printf("%s\n", word);
                    numberOfWords--;

                    *wordsp = (char*)malloc((strlen(word) + 1) * sizeof(char));
                    strcpy(*wordsp, word);
                    wordsp++;

                    generatedCount++;
                }
                word[0] = '\0';
            }
        }
    }

    free(words);
}