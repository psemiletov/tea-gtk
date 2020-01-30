#ifndef LATEXSUPPORT_H
#define LATEXSUPPORT_H

void latex_process (const gchar *filename);
void latex_dvi_to_ps (const gchar *filename);
void latex_dvi_to_pdf (const gchar *filename);
void view_dvi (const gchar *filename);
void view_ps (const gchar *filename);
void view_pdf (const gchar *filename);
void pdflatex_process (const gchar *filename);

#endif