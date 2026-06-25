



/* This is the representation of the expressions to determine the
   plural form.  */
struct expression
{
  int nargs;			/* Number of arguments.  */
  union
  {
    unsigned long int num;	/* Number value for `num'.  */
    struct expression *args[3];	/* Up to three arguments.  */
  } val;
};


struct expression GERMANIC_PLURAL =
{
  .nargs = 2,
  .val =
  {
    .args =
    {
      [0] = (struct expression *) &plvar,
      [1] = (struct expression *) &plone
    }
  }
};

