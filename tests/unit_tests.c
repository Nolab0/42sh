#include <criterion/criterion.h>
#include <criterion/redirect.h>

#include "test.h"

void redirect_stdout(void)
{
    cr_redirect_stdout();
}

Test(FirstTest, coucou)
{
    redirect_stdout();
    print_coucou();
    cr_assert_stdout_eq_str("coucou\n");
}
