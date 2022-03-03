TEMPLATE = subdirs

test_projects = $$files(*.pro)
test_projects -= tests.pro

for(test_pro, test_projects) {
    test_name = $$replace(test_pro, .pro, )
    eval($${test_name}.file = $${test_pro})
    SUBDIRS += $${test_name}
}
