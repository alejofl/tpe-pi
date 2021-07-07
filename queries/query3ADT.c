#include "./query3ADT.h"

typedef struct Recording {
    String title;
    size_t votes;
    float rating;
} Recording;

typedef struct year {
    size_t year;
    Recording maxMovie;
    Recording maxSerie;
    struct year * next;
} Year;

typedef struct query3CDT {
    size_t size;
    Year * years;
} query3CDT;

query3ADT newQuery3(ERROR_CODE * err) {
    errno = 0;
    query3ADT q = calloc(1, sizeof(struct query3CDT));
    if (errno == ENOMEM) {
        *err = MEM_ERROR;
        return NULL;
    }
    *err = NO_ERROR;
    return q;
}

static Year * insertYearRec(Year * list, Entry * m, ERROR_CODE * err, int * flag) {
    //Caso 1: no hay nada en la lista o no está el año -> esa va a ser la entrada con más votos hasta ahora
    if (list == NULL || list->year < m->startYear) {
        errno = 0;
        Year * newNode = malloc(sizeof(Year));
        if (errno == ENOMEM) {
            *err = MEM_ERROR;
            return list;
        }
        newNode->year = m->startYear;
        if (strcasecmp("movie", m->titleType) == 0) {
            newNode->maxMovie.title = copyStr(m->primaryTitle);
            if (newNode->maxMovie.title == NULL) {
                *err = MEM_ERROR;
                return list;
            }
            newNode->maxMovie.rating = m->averageRating;
            newNode->maxMovie.votes = m->numVotes;
            newNode->maxSerie = (Recording){.title = NULL, .rating = 0, .votes = 0};
        } else {
            newNode->maxSerie.title = copyStr(m->primaryTitle);
            if (newNode->maxSerie.title == NULL) {
                *err = MEM_ERROR;
                return list;
            }
            newNode->maxSerie.rating = m->averageRating;
            newNode->maxSerie.votes = m->numVotes;
            newNode->maxMovie = (Recording){.title = NULL, .rating = 0, .votes = 0};
        }
        newNode->next = list;
        *flag = 1;
        *err = NO_ERROR;
        return newNode;
    }
    //Caso 2: el año de la lista es igual al de la entrada -> me tengo que fijar si tiene más votos.
    if (list->year == m->startYear) {
        if (strcasecmp("movie", m->titleType) == 0) {
            if (list->maxMovie.votes < m->numVotes) {
                free(list->maxMovie.title);
                list->maxMovie.title = copyStr(m->primaryTitle);
                if (list->maxMovie.title == NULL) {
                    *err = MEM_ERROR;
                    return list;
                }
                list->maxMovie.rating = m->averageRating;
                list->maxMovie.votes = m->numVotes;
            }
        } else {
            if (list->maxSerie.votes < m->numVotes) {
                free(list->maxSerie.title);
                list->maxSerie.title = copyStr(m->primaryTitle);
                if (list->maxSerie.title == NULL) {
                    *err = MEM_ERROR;
                    return list;
                }
                list->maxSerie.rating = m->averageRating;
                list->maxSerie.votes = m->numVotes;
            }
        }
        *err = NO_ERROR;
        return list;
    }
    //Caso 3: el año es menor que el de list -> me fijo en next.
    list->next = insertYearRec(list->next, m, err, flag);
    return list;
}

void insertQ3(query3ADT q, Entry * m, ERROR_CODE * err) {
    int flag = 0;
    q->years = insertYearRec(q->years, m, err, &flag);
    q->size += flag;
}

void freeFinalVecQ3(DataQ3 * vec, size_t dim) {
    for (int i = 0; i < dim; i++) {
        free(vec[i].serieTitle);
        free(vec[i].movieTitle);
    }
    free(vec);
}

DataQ3 * finalVecQ3(query3ADT q, ERROR_CODE * err) {
    errno = 0;
    DataQ3 * vec = malloc(q->size * sizeof(DataQ3));
    if (errno == ENOMEM) {
        *err = MEM_ERROR;
        return NULL;
    }
    Year * auxList = q->years;
    for (int i = 0; i < q->size; i++) {
        vec[i].year = auxList->year;
        vec[i].movieRating = auxList->maxMovie.rating;
        vec[i].movieVotes = auxList->maxMovie.votes;
        vec[i].serieRating = auxList->maxSerie.rating;
        vec[i].serieVotes = auxList->maxSerie.votes;
        if (auxList->maxMovie.title == NULL) {
            vec[i].movieTitle = copyStr("\\N");
        } else {
            vec[i].movieTitle = copyStr(auxList->maxMovie.title);
        }
        if (vec[i].movieTitle == NULL) {
            *err = MEM_ERROR;
            freeFinalVecQ3(vec, i-1);
            return NULL;
        }
        if (auxList->maxSerie.title == NULL) {
            vec[i].serieTitle = copyStr("\\N");
        } else {
            vec[i].serieTitle = copyStr(auxList->maxSerie.title);
        }
        if (vec[i].serieTitle == NULL) {
            *err = MEM_ERROR;
            freeFinalVecQ3(vec, i-1);
            return NULL;
        }
        auxList = auxList->next;
    }
    return vec;
}

static void freeRec(Year * list) {
    if (list == NULL) {
        return;
    }
    free(list->maxMovie.title);
    free(list->maxSerie.title);
    freeRec(list->next);
    free(list);
}

void freeQueryQ3(query3ADT q) {
    freeRec(q->years);
    free(q);
}

size_t q3Size(query3ADT q) {
    return q->size;
}
