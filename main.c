#include "basics.h"

#include "reader.h"
#include "source.h"
#include "token.h"
#include "tokenize.h"
#include "ast.h"
#include "parse.h"
#include "print_ast.h"
#include "flatten_control_flow.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Please provide file to compile.\n");
    exit(1);
  }

  struct Source source;
  if (!read_file(argv[1], &source)) {
    exit(1);
  }
  {
    struct Reader reader = {
        .source = source,
        .position = {1, 1},
    };
    struct TokenStream stream = {
        .reader = reader,
    };
    while (true) {
      token_stream_advance(&stream);
      struct Token token = stream.current;
      if (token.type == TK_EOF) {
        printf("EOF\n");
        break;
      }
      if (token.type == TK_ERROR) {
        printf("ERROR\n");
        break;
      }
      if (token.type == TK_IDENT) {
        printf("IDENT ");
      } else if (token.type == TK_EQ) {
        printf("EQ ");
      } else if (token.type == TK_NE) {
        printf("NE ");
      } else if (token.type == TK_INT) {
        printf("INT ");
      } else if (token.type == TK_ARROW) {
        printf("ARROW ");
      } else if (token.type >= 33 && token.type <= 126) {
        printf("SIGIL ");
      }
      printf("\"");
      print_from_source(&source, token.start, token.length);
      printf("\"");
      printf("\n");
    }
  }
  {
    struct Reader reader = {
        .source = source,
        .position = {1, 1},
    };
    struct TokenStream stream = {
        .reader = reader,
    };
    token_stream_advance(&stream);
    struct AstTl *ast;
    if (!parse_all_tl(&stream, &ast)) {
      return -1;
    }
    print_tl_all(ast);
    flatten_all_functions(ast);
    print_tl_all(ast);
  }
}
