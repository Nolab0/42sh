#include <criterion/criterion.h>
#include <criterion/redirect.h>

#include "parser.h"
#include "ast.h"

void redirect_stdout(void)
{
    cr_redirect_stdout();
}

Test(FirstTest, coucou)
{
    redirect_stdout();
    print_coucou();
    fflush(stdout);
    cr_assert_stdout_eq_str("coucou\n");
}
