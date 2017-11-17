#include <linux/module.h>
#include <linux/init.h>
#include <linux/oom.h>
#include <linux/notifier.h>
#include <linux/kobject.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jonathan 'hachi' Steinert");
MODULE_DESCRIPTION("Count oom handlings");

atomic_t ooms = ATOMIC_INIT(0);
static struct kobject *oomph_kobj;

int oom_handler(struct notifier_block *self, unsigned long argc, void * argv) {
    atomic_inc(&ooms);
    return NOTIFY_OK;
}

struct notifier_block nb = {
    .notifier_call = &oom_handler,
};

static ssize_t ooms_show(struct kobject *kobj, struct kobj_attribute *attr,
                    char *buf)
{
        return sprintf(buf, "%u\n", atomic_read(&ooms));
}

static struct kobj_attribute ooms_attribute =
    __ATTR(ooms, 0444, ooms_show, NULL);


/*
 *  * Create a group of attributes so that we can create and destroy them all
 *   * at once.
 *    */
static struct attribute *attrs[] = {
    &ooms_attribute.attr,
    NULL,   /* need to NULL terminate the list of attributes */
};

/*
 *  * An unnamed attribute group will put all of the attributes directly in
 *   * the kobject directory.  If we specify a name, a subdirectory will be
 *    * created for the attributes with the directory being the name of the
 *     * attribute group.
 *      */
static struct attribute_group attr_group = {
    .attrs = attrs,
};


static int __init oomph_init(void)
{
    int retval;

    oomph_kobj = kobject_create_and_add("oomph", kernel_kobj);
    if (!oomph_kobj)
        return -ENOMEM;

    /* Create the files associated with this kobject */
    retval = sysfs_create_group(oomph_kobj, &attr_group);
    if (retval)
        kobject_put(oomph_kobj);

    register_oom_notifier(&nb);

    return retval;
}

static void __exit oomph_exit(void)
{
    kobject_put(oomph_kobj);

    unregister_oom_notifier(&nb);
}

module_init(oomph_init);
module_exit(oomph_exit);
