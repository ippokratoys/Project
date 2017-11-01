SOURCE = ngrams.c trie.c trie_node.c tools.c
OBJS = ngrams.o trie.o trie_node.o tools.o
EXEC =NGrams.exe
CC	= gcc
FLAGS   = -g -c
FILES_FOLDER = test_files
all: clean $(EXEC)

test_file: all
	time ./$(EXEC) -i $(FILES_FOLDER)/$(file).init -q $(FILES_FOLDER)/$(file).work > out << -EOF
	diff ./out $(FILES_FOLDER)/$(file).result


$(EXEC): $(OBJS)
	$(CC) -g $? -o $@

ngrams.o: ngrams.c
	$(CC) $(FLAGS) $?

trie.o: trie.c
	$(CC) $(FLAGS) $?

trie_node.o: trie_node.c
	$(CC) $(FLAGS) $?

tools.o: tools.c
	$(CC) $(FLAGS) $?

test: test_entry
	./test_entry

test_entry: test_entry.o trie.o trie_node.o tools.o
	$(CC) -o $@ $?

test_entry.o: test_entry.c
	$(CC) $(FLAGS) $?

clean:
	rm -rf $(EXEC) $(OBJS)
