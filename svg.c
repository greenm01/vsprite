// ======================================================================
// tom's SVG path parser  2010-04-03
//
// - Simple top-down parser/lexer in the style of 'META II' [Schorre 1964]
//

#include "svg.h"

//========================================================================

#include <ctype.h>
#include <stdio.h>
//#include "uthash/utstring.h"
#include "uthash/uthash.h"

// XXX CRUFT...
const float32 kMaxShipSize = 5;  // Maximum ship size (meters)
const int kCirclePoints = 25;   // Number of points in a circle
const int kBezierPoints = 20;   // Number of points in a Bezier curve

//========================================================================

// Module-global state (nope, this isn't reentrant/threadsafe...)
FILE *input;
bool svg_debug = true;
float32 width, height;


// Little parser subroutines...

int fpeek(FILE *f) {
    int c = fgetc(f);
      ungetc(c, f);
        return c;
}

void skip_whitespace() {
    while( !feof(input) && isspace(fpeek(input)) )
          fgetc(input);
}

bool is_sym(char c) {
    return isalnum(c) || c=='"' || c=='\'' || c==':' || c=='.'
          || c=='*' || c=='/' || c=='%' || c=='+' || c=='-' || c=='_';
}

// Match a string
bool match(char *s) {
  long pos = ftell(input);
  int n = strlen(s);
  int i;
  for(i=0; i<n; i++) {
    if( s[i] != fgetc(input) ) {
      fseek(input, pos, SEEK_SET);
      return false;
    }
  }
  return true;
}

//========================================================================
// SVG tag parser subroutines
//========================================================================

//------------------------------------------------------------------------
// Traverse the XML element tree
//
void parse_svg() {
  /*
  //const char* name = node.getName();
  //const char* transform = node.getAttribute("transform");

  Matrix *old_transform = transform_;
  //transform_ = mul(transform_, new Matrix(transform?std::string(transform):std::string("")));

  // Parse current node
  fnmap::iterator iter;
  iter = vtable.find(name);
  if(iter != vtable.end()) {
    // Dispatch to the appropriate tag-handler
    (this->*(iter->second))(node);
  } else {
    //TODO debug log...
    if(svg_debug) printf("Ignoring '%s' element\n", name);
  }

  // Apply transform
  if(path) {
    path->transform = transform_;
    path->apply_transform();
    path->id = (char*) node.getAttribute("id");
    color_style_(node);
    path_list.push_back(path);
  }

  transform_ = old_transform;

  // Process child nodes
  int num_children = node.nChildNode();
  for(int i = 0; i < num_children; i++) {
    XMLNode child = node.getChildNode(i);
    parse_svg(child, indent+1);
  }

  path = NULL;  //TODO this is now 'curpath' in path.c
  */
}

//========================================================================
// Hash table
//========================================================================

typedef void(*fnptr)();         // Function Pointer

// hash map for looking up XML tag names
typedef struct {
  char *name;   // key (use HASH_ADD_KEYPTR)
  fnptr fn;     // value
  UT_hash_handle hh;
} Fnmap;
Fnmap *fnmap = NULL;

void fnmap_add(char *name, fnptr fn) {
  Fnmap *s = malloc(sizeof(Fnmap));
  s->name = name;
  s->fn = fn;
  HASH_ADD_KEYPTR( hh, fnmap, s->name, strlen(s->name), s );
}

void init_fnmap() {
  fnmap_add("svg", parse_svg);
  //fnmap_add("path", parse_path_);
  //fnmap_add("rect", parse_rect_);
  //fnmap_add("circle", parse_circle_);
  //fnmap_add("radialGradient", parse_radial_gradient_);
  //fnmap_add("linearGradient", parse_linear_gradient_);
}

//========================================================================
// XML parsing
//========================================================================

bool parse_xml_comment() {
  //comment = '<!--' ([^-] | '-' . [^-])* '-->'     IGNORE
  if(!match("<!--")) return false;
  for(;;) {
    int c = fgetc(input);
    if(c=='-') {
      if(match("->")) {
        printf("PARSED A COMMENT\n");
        return true;
      }
    }
  }
}

//------------------------------------------------------------------------
char* parse_xml_name() {
  char *buf;
  size_t bufsize;
  FILE *out = open_memstream(&buf, &bufsize);
  if (out == NULL) {
    perror("open_memstream");
    exit(1);
  }
  int c = fgetc(input);
  //printf("--> '%c'\n", c);

  if (isalpha(c) || c=='_' || c==':') {
    // Other chars (_ and : aren't
    while (isalnum(c) || c=='_' || c==':' || c=='.' || c=='-') {
      fputc(c, out);
      c = fgetc(input);
      //printf("--> '%c'\n", c);
    }
    ungetc(c, input);
    fclose(out);
    //printf("Parsed tag name <%s>\n", buf);
    return buf;
  }
  else {
    ungetc(c, input);
    fclose(out);
    free(buf);
    return NULL;
  }
}

// Parse over (skip) extraneous data
bool skip_attrs() {
  skip_whitespace();
  if(parse_xml_name() == NULL) {
    return false;
  }
  skip_whitespace();
  // '='
  int c = fgetc(input);
  skip_whitespace();
  // '"'
  c = fgetc(input);
  for(;;) {
    c = fgetc(input);
    if (c == '"') break;
  }
  return true;
}

//------------------------------------------------------------------------
// Parse and return xml attribute name and value
bool parse_xml_attr(char **name, char **value) {
  skip_whitespace();

  // attribute name
  char *k = parse_xml_name();
  if(k == NULL) {
    return false;
  }

  skip_whitespace();

  // '='
  int c = fgetc(input);
  if (c != '=') {
    printf("EXPECTED '='\n");
    return false;
  }

  skip_whitespace();

  char *buf;
  size_t bufsize;
  FILE *out = open_memstream(&buf, &bufsize);
  if (out == NULL) {
    perror("open_memstream");
    exit(1);
  }

  // attribute value (quoted string)
  c = fgetc(input);
  switch (c) {
    case '"':
      for(;;) {
        c = fgetc(input);
        //TODO parse &...; XML entity refs
        if (c == '"') break;
        fputc(c, out);
      }
      break;
    case '\'':
      for(;;) {
        c = fgetc(input);
        //TODO parse &...; XML entity refs
        if (c == '\'') break;
        fputc(c, out);
      }
      break;
    default:
      fclose(out); free(buf);
      printf("EXPECTED xml attr value (quoted string)\n");
      return false;
  }
  fclose(out);
  
  *name = k;
  *value = buf;
  
  return true;
}

bool parse_svg_path(Sprite *sprite, Path *path) {
  
  char *name, *value;
  while(parse_xml_attr(&name, &value));
  
}

//------------------------------------------------------------------------
bool parse_xml_attrs(Sprite *sprite, const char *element) {
  
  char *name, *value;
  
  // Extract height and width data from svg
  if(!strcmp(element, "svg")) {
    while(parse_xml_attr(&name, &value)) {
      if(!strcmp(name, "width")) {
        sprite->width = atof(value);
      } else if (!strcmp(name, "height")) {
        sprite->height = atof(value);
      }
    }
  } else if(!strcmp(element, "g")) {
    // Extract skeleton from group
    // Maybe there's a smarter way to do the skeleton rather than manual
    while(parse_xml_attr(&name, &value)) {
      if(!strcmp(name, "id")) {
        if(!strcmp(value, "skeleton")) {
          printf("**** found the skeleton! ****\n");
        }
      }
    }
  } else if(!strcmp(element, "path")) {
      Path *path = path_new();
      parse_svg_path(sprite, path);
  } else {
    // Parse and skip everything else
    while(skip_attrs());
  }
  
  return true;
}

//------------------------------------------------------------------------
bool parse_xml_pi() {
  // Parse a program instruction ("<?... ?>") and IGNORE IT
  if(!match("<?")) return false;
  //printf("GOT <?\n");
  free(parse_xml_name());
  //printf("GOT name\n");
  while(skip_attrs());
  //printf("GOT attrs\n");
  if(!match("?>")) return false;
  //printf("GOT ?>\n");
  return true;
}

//------------------------------------------------------------------------
bool parse_xml_misc() {
  // Parse "misc" - comments and PIs
  do skip_whitespace();
  while(parse_xml_pi() || parse_xml_comment());
  return true;
}

//------------------------------------------------------------------------
bool parse_xml_prolog() {
  // <?xml ... ?> header
  if (!parse_xml_pi()) return false;
  // Comments and PIs
  parse_xml_misc();
  return true;
}

//------------------------------------------------------------------------
bool parse_xml_element();  // forward ref

bool parse_xml_content() {
  for(;;) {
    int c = fgetc(input);
    if(c=='<') {
      int c = fgetc(input);
      fseek(input, -2, SEEK_CUR);
      if(c=='/') {
        return true;
      }
      parse_xml_element();
    }
  }
}

//------------------------------------------------------------------------
bool parse_xml_element(Sprite *sprite) {
  if(fgetc(input) != '<') return false;
  char *element = parse_xml_name();
  printf("BEGIN <%s> TAG\n", element);

  parse_xml_attrs(sprite, element);

  if(match("/>")) {
    printf("PARSED EMPTY <%s/> TAG\n", element);
    return true;
  }

  if(!match(">")) {
    printf("EXPECTED '>' TO CLOSE <%s> TAG\n", element);
    return false;
  }

  printf("PARSING CONTENT OF <%s> TAG\n", element);
  parse_xml_content();

  if(! (match("</") && match(element) && match(">"))) {
    printf("EXPECTED </%s> CLOSING TAG\n", element);
    return false;
  }
  return true;
}

//------------------------------------------------------------------------
// Parses the given SVG file and stores it in the supplied Sprite.
// Returns true on success.
//
bool svg_load(const char *filename, float32 scale, Sprite *sprite) {
  if(svg_debug) {
    printf("===============================================================\n");
    printf("Loading %s  scale=%f\n", filename, scale);
    printf("===============================================================\n");
  }

  //TODO new XML parser....
  //XMLNode top = XMLNode::openFileHelper(filename).getChildNode("svg");
  input = fopen(filename, "r");
  if (!input) {
    perror("unable to open input file");
    return false;
  }
  if (!(parse_xml_prolog())) {
    printf("XML prologue not parsed\n");
    return false;
  }
  if (!parse_xml_element(sprite)) {
    printf("FAILED to parse XML body (i.e. <svg>...</svg>\n");
    return false;
  }
  printf("FINISHED PARSING SVG\n");

#if 0
  const char *width_s = top.getAttribute("width");
  const char *height_s = top.getAttribute("height");

  double width = atof(width_s);
  double height = atof(height_s);

  // This flips from SVG cord. space to OpenGL/world cord. space
  transform_ = new Matrix(1, 0, 0, -1, 0, height);

  printf("w = %f, h = %f\n", width, height);

  // Parse SVG paths into a temporary list
  path_list.clear();
  parse_svg(top, 0);

  skin->path_list = path_list;

  // Render to a GL display list
  GLuint displist = glGenLists(1);
  if(!displist) {
    std::cerr << "WARNING: Could not allocate displist in SVGparser::load()\n";
  }
  else {
    glNewList(displist, GL_COMPILE);
      glRotatef(180, 0, 0, 1);
      glScaled(scale, scale, scale);
      PathList::iterator x = skin->path_list.begin();
      while(x != skin->path_list.end()) {
        (*x)->render();
        ++x;
      }
    glEndList();
  }
  skin->displist = displist;
#endif

  return true;
}
//========================================================================
