/*
 * Linux device driver for
 * Broadcom BCM47XX 10/100/1000 Mbps Ethernet Controller
 *
 * Copyright (C) 2012, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: et_linux.c 350756 2012-08-15 10:15:48Z $
 */

#include <et_cfg.h>
#define __UNDEF_NO_VERSION__

#include <typedefs.h>

#include <linux/module.h>
#include <linuxver.h>
#include <bcmdefs.h>
#include <osl.h>

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/sockios.h>
#ifdef SIOCETHTOOL
#include <linux/ethtool.h>
#endif /* SIOCETHTOOL */
#include <linux/ip.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/pgtable.h>
#include <asm/uaccess.h>

#include <epivers.h>
#include <bcmendian.h>
#include <bcmdefs.h>
#include <proto/ethernet.h>
#include <proto/vlan.h>
#include <proto/bcmip.h>
#include <bcmdevs.h>
#include <bcmenetmib.h>
#include <bcmgmacmib.h>
#include <bcmenetrxh.h>
#include <bcmenetphy.h>
#include <etioctl.h>
#include <bcmutils.h>
#include <pcicfg.h>
#include <et_dbg.h>
#include <hndsoc.h>
#include <bcmgmacrxh.h>
#include <etc.h>
#ifdef HNDCTF
#include <ctf/hndctf.h>
#endif /* HNDCTF */

#ifdef ET_ALL_PASSIVE_ON
/* When ET_ALL_PASSIVE_ON, ET_ALL_PASSIVE must be true */
#define ET_ALL_PASSIVE_ENAB(et)	1
#else
#ifdef ET_ALL_PASSIVE
#define ET_ALL_PASSIVE_ENAB(et)	(!(et)->all_dispatch_mode)
#else /* ET_ALL_PASSIVE */
#define ET_ALL_PASSIVE_ENAB(et)	0
#endif /* ET_ALL_PASSIVE */
#endif	/* ET_ALL_PASSIVE_ON */

#ifdef ET_ALL_PASSIVE
#define ET_LIMIT_TXQ
#endif

#ifdef PKTC
#ifndef HNDCTF
#error "Packet chaining feature can't work w/o CTF"
#endif
#define PKTC_ENAB(et)	((et)->etc->pktc)
#define PKT_CHAINABLE(et, p, evh, prio, h_sa, h_da, h_prio) \
	(!eacmp((h_da), ((struct ethervlan_header *)(evh))->ether_dhost) && \
	 !eacmp((h_sa), ((struct ethervlan_header *)(evh))->ether_shost) && \
	 (et)->brc_hot && CTF_HOTBRC_CMP((et)->brc_hot, (evh), (void *)(et)->dev) && \
	 ((h_prio) == (prio)) && !RXH_FLAGS((et)->etc, PKTDATA((et)->osh, (p))) && \
	 (((struct ethervlan_header *)(evh))->vlan_type == HTON16(ETHER_TYPE_8021Q)) && \
	 ((((struct ethervlan_header *)(evh))->ether_type == HTON16(ETHER_TYPE_IP)) || \
	 (((struct ethervlan_header *)(evh))->ether_type == HTON16(ETHER_TYPE_IPV6))))
#define PKTCMC	2
struct pktc_data {
	void	*chead;		/* chain head */
	void	*ctail;		/* chain tail */
	uint8	*h_da;		/* pointer to da of chain head */
	uint8	*h_sa;		/* pointer to sa of chain head */
	uint8	h_prio;		/* prio of chain head */
};
typedef struct pktc_data pktc_data_t;
#else /* PKTC */
#define PKTC_ENAB(et)	0
#define PKT_CHAINABLE(et, p, evh, prio, h_sa, h_da, h_prio) 	0
#endif /* PKTC */

static int et_get_settings(struct net_device *dev, struct ethtool_cmd *ecmd);
static int et_set_settings(struct net_device *dev, struct ethtool_cmd *ecmd);
static void et_get_driver_info(struct net_device *dev, struct ethtool_drvinfo *info);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
static const struct ethtool_ops et_ethtool_ops =
{
	.get_settings = et_get_settings,
	.set_settings = et_set_settings,
	.get_drvinfo = et_get_driver_info
};
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36) */


MODULE_LICENSE("Proprietary");

/* In 2.6.20 kernels work functions get passed a pointer to the
 * struct work, so things will continue to work as long as the work
 * structure is the first component of the task structure.
 */
typedef struct et_task {
	struct work_struct work;
	void *context;
} et_task_t;

typedef struct et_info {
	etc_info_t	*etc;		/* pointer to common os-independent data */
	struct net_device *dev;		/* backpoint to device */
	struct pci_dev *pdev;		/* backpoint to pci_dev */
	void		*osh;		/* pointer to os handle */
#ifdef HNDCTF
	ctf_t		*cih;		/* ctf instance handle */
	ctf_brc_hot_t	*brc_hot;	/* hot bridge cache entry */
#endif
	struct semaphore sem;		/* use semaphore to allow sleep */
	spinlock_t	lock;		/* per-device perimeter lock */
	spinlock_t	txq_lock;	/* lock for txq protection */
	struct sk_buff_head txq[NUMTXQ];	/* send queue */
	void *regsva;			/* opaque chip registers virtual address */
	struct timer_list timer;	/* one second watchdog timer */
	bool set;			/* indicate the timer is set or not */
	struct net_device_stats stats;	/* stat counter reporting structure */
	int events;			/* bit channel between isr and dpc */
	struct et_info *next;		/* pointer to next et_info_t in chain */
#ifdef	NAPI2_POLL
	struct napi_struct napi_poll;
#endif	/* NAPI2_POLL */
#ifndef NAPI_POLL
	struct tasklet_struct tasklet;	/* dpc tasklet */
#endif /* NAPI_POLL */
#ifdef ET_ALL_PASSIVE
	et_task_t	dpc_task;	/* work queue for rx dpc */
	et_task_t	txq_task;	/* work queue for tx frames */
	et_task_t	wd_task;	/* work queue for watchdog */
	bool		all_dispatch_mode;	/* dispatch mode: tasklets or passive */
#endif /* ET_ALL_PASSIVE */
	bool resched;			/* dpc was rescheduled */
} et_info_t;

static int et_found = 0;
static et_info_t *et_list = NULL;

/* defines */
#define	DATAHIWAT	1000		/* data msg txq hiwat mark */

#define	ET_INFO(dev)	(et_info_t *)(DEV_PRIV(dev))


#define ET_LOCK(et) \
do { \
	if (ET_ALL_PASSIVE_ENAB(et)) \
		down(&(et)->sem); \
	else \
		spin_lock_bh(&(et)->lock); \
} while (0)

#define ET_UNLOCK(et) \
do { \
	if (ET_ALL_PASSIVE_ENAB(et)) \
		up(&(et)->sem); \
	else \
		spin_unlock_bh(&(et)->lock); \
} while (0)

#define ET_TXQ_LOCK(et)		spin_lock_bh(&(et)->txq_lock)
#define ET_TXQ_UNLOCK(et)	spin_unlock_bh(&(et)->txq_lock)

#define INT_LOCK(flags)		local_irq_save(flags)
#define INT_UNLOCK(flags)	local_irq_restore(flags)

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 4, 5)
#error Linux version must be newer than 2.4.5
#endif	/* LINUX_VERSION_CODE <= KERNEL_VERSION(2, 4, 5) */

/* linux 2.4 doesn't have in_atomic */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20)
#define in_atomic() 0
#endif

/* prototypes called by etc.c */
void et_init(et_info_t *et, uint options);
void et_reset(et_info_t *et);
void et_link_up(et_info_t *et);
void et_link_down(et_info_t *et);
void et_up(et_info_t *et);
void et_down(et_info_t *et, int reset);
void et_dump(et_info_t *et, struct bcmstrbuf *b);
#ifdef HNDCTF
void et_dump_ctf(et_info_t *et, struct bcmstrbuf *b);
#endif

/* local prototypes */
static void et_free(et_info_t *et);
static int et_open(struct net_device *dev);
static int et_close(struct net_device *dev);
static int et_start(struct sk_buff *skb, struct net_device *dev);
static void et_sendnext(et_info_t *et);
static struct net_device_stats *et_get_stats(struct net_device *dev);
static int et_set_mac_address(struct net_device *dev, void *addr);
static void et_set_multicast_list(struct net_device *dev);
static void _et_watchdog(struct net_device *data);
static void et_watchdog(ulong data);
static int et_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
static irqreturn_t et_isr(int irq, void *dev_id);
#else
static irqreturn_t et_isr(int irq, void *dev_id, struct pt_regs *ptregs);
#endif
#ifdef	NAPI2_POLL
static int et_poll(struct napi_struct *napi, int budget);
#elif defined(NAPI_POLL)
static int et_poll(struct net_device *dev, int *budget);
#else /* ! NAPI_POLL */
static void et_dpc(ulong data);
#endif /* NAPI_POLL */
#ifdef ET_ALL_PASSIVE
static void et_dpc_work(struct et_task *task);
static void et_txq_work(struct et_task *task);
static void et_watchdog_task(et_task_t *task);
#endif /* ET_ALL_PASSIVE */
static void et_sendup(et_info_t *et, struct sk_buff *skb);
#ifdef BCMDBG
static void et_dumpet(et_info_t *et, struct bcmstrbuf *b);
#endif /* BCMDBG */

#ifdef HAVE_NET_DEVICE_OPS
static const struct net_device_ops et_netdev_ops = {
	.ndo_open = et_open,
	.ndo_stop = et_close,
	.ndo_start_xmit = et_start,
	.ndo_get_stats = et_get_stats,
	.ndo_set_mac_address = et_set_mac_address,
	.ndo_set_multicast_list = et_set_multicast_list,
	.ndo_do_ioctl = et_ioctl,
};
#endif /* HAVE_NET_DEVICE_OPS */

/* recognized PCI IDs */
static struct pci_device_id et_id_table[] __devinitdata = {
	{ vendor: PCI_ANY_ID,
	device: PCI_ANY_ID,
	subvendor: PCI_ANY_ID,
	subdevice: PCI_ANY_ID,
	class: PCI_CLASS_NETWORK_OTHER << 8,
	class_mask: 0xffff00,
	driver_data: 0,
	},
	{ vendor: PCI_ANY_ID,
	device: PCI_ANY_ID,
	subvendor: PCI_ANY_ID,
	subdevice: PCI_ANY_ID,
	class: PCI_CLASS_NETWORK_ETHERNET << 8,
	class_mask: 0xffff00,
	driver_data: 0,
	},
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, et_id_table);

#if defined(BCMDBG)
static uint32 msglevel = 0xdeadbeef;
module_param(msglevel, uint, 0644);
#endif /* defined(BCMDBG) */

#ifdef ET_ALL_PASSIVE
/* passive mode: 1: enable, 0: disable */
static int passivemode = 0;
module_param(passivemode, int, 0);
#endif /* ET_ALL_PASSIVE */
#ifdef ET_LIMIT_TXQ
#define ET_TXQ_THRESH	0
static int et_txq_thresh = ET_TXQ_THRESH;
module_param(et_txq_thresh, int, 0);
#endif /* ET_LIMIT_TXQ */

#ifdef HNDCTF
static void
et_ctf_detach(ctf_t *ci, void *arg)
{
	et_info_t *et = (et_info_t *)arg;

	et->cih = NULL;

#ifdef CTFPOOL
	/* free the buffers in fast pool */
	osl_ctfpool_cleanup(et->osh);
#endif /* CTFPOOL */

	return;
}
#endif /* HNDCTF */

static int __devinit
et_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	struct net_device *dev = NULL;
	et_info_t *et;
	osl_t *osh;
	char name[128];
	int i, unit = et_found, err;

	ET_TRACE(("et%d: et_probe: bus %d slot %d func %d irq %d\n", unit,
	          pdev->bus->number, PCI_SLOT(pdev->devfn), PCI_FUNC(pdev->devfn), pdev->irq));

	if (!etc_chipmatch(pdev->vendor, pdev->device))
		return -ENODEV;

	et_found++;

	/* pre-qualify et unit, that can save the effort to do et_detach */
	sprintf(name, "et%dmacaddr", unit);
	if (getvar(NULL, name) == NULL) {
		ET_ERROR(("et%d: et%dmacaddr not found, ignore it\n", unit, unit));
		return -ENODEV;
	}

	osh = osl_attach(pdev, PCI_BUS, FALSE);
	ASSERT(osh);

	pci_set_master(pdev);
	if ((err = pci_enable_device(pdev)) != 0) {
		ET_ERROR(("et%d: et_probe: pci_enable_device() failed\n", unit));
		osl_detach(osh);
		return err;
	}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	if ((dev = alloc_etherdev(sizeof(et_info_t))) == NULL) {
		ET_ERROR(("et%d: et_probe: alloc_etherdev() failed\n", unit));
		osl_detach(osh);
		return -ENOMEM;
	}
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
	if ((dev = alloc_etherdev(0)) == NULL) {
		ET_ERROR(("et%d: et_probe: alloc_etherdev() failed\n", unit));
		osl_detach(osh);
		return -ENOMEM;
	}
	/* allocate private info */
	if ((et = (et_info_t *)MALLOC(osh, sizeof(et_info_t))) == NULL) {
		ET_ERROR(("et%d: et_probe: out of memory, malloced %d bytes\n", unit,
		          MALLOCED(osh)));
		MFREE(osh, dev, sizeof(et_info_t));
		osl_detach(osh);
		return -ENOMEM;
	}
	dev->priv = (void *)et;
#else
	if (!(dev = (struct net_device *) MALLOC(osh, sizeof(struct net_device)))) {
		ET_ERROR(("et%d: et_probe: out of memory, malloced %d bytes\n", unit,
		          MALLOCED(osh)));
		osl_detach(osh);
		return -ENOMEM;
	}
	bzero(dev, sizeof(struct net_device));

	if (!init_etherdev(dev, 0)) {
		ET_ERROR(("et%d: et_probe: init_etherdev() failed\n", unit));
		MFREE(osh, dev, sizeof(struct net_device));
		osl_detach(osh);
		return -ENOMEM;
	}
	/* allocate private info */
	if ((et = (et_info_t *)MALLOC(osh, sizeof(et_info_t))) == NULL) {
		ET_ERROR(("et%d: et_probe: out of memory, malloced %d bytes\n", unit,
		          MALLOCED(osh)));
		MFREE(osh, dev, sizeof(et_info_t));
		osl_detach(osh);
		return -ENOMEM;
	}
	dev->priv = (void *)et;
#endif /* < 2.6.0 */

	et = ET_INFO(dev);
	bzero(et, sizeof(et_info_t));	/* Is this needed in 2.6.36 ? -LR */
	et->dev = dev;
	et->pdev = pdev;
	et->osh = osh;
	pci_set_drvdata(pdev, et);

	/* map chip registers (47xx: and sprom) */
	dev->base_addr = pci_resource_start(pdev, 0);
	if ((et->regsva = ioremap_nocache(dev->base_addr, PCI_BAR0_WINSZ)) == NULL) {
		ET_ERROR(("et%d: ioremap() failed\n", unit));
		goto fail;
	}

	init_MUTEX(&et->sem);
	spin_lock_init(&et->lock);
	spin_lock_init(&et->txq_lock);

	for (i = 0; i < NUMTXQ; i++)
		skb_queue_head_init(&et->txq[i]);

	/* common load-time initialization */
	et->etc = etc_attach((void *)et, pdev->vendor, pdev->device, unit, osh, et->regsva);
	if (et->etc == NULL) {
		ET_ERROR(("et%d: etc_attach() failed\n", unit));
		goto fail;
	}

#ifdef HNDCTF
	et->cih = ctf_attach(osh, dev->name, &et_msg_level, et_ctf_detach, et);

	if (ctf_dev_register(et->cih, dev, FALSE) != BCME_OK) {
		ET_ERROR(("et%d: ctf_dev_register() failed\n", unit));
		goto fail;
	}
#endif /* HNDCTF */

#ifdef CTFPOOL
	/* create ctf packet pool with specified number of buffers */
	if (CTF_ENAB(et->cih)) {
		uint32 poolsz;
		/* use large ctf poolsz for platforms with more memory */
		poolsz = ((num_physpages >= 32767) ? CTFPOOLSZ * 2 :
		          ((num_physpages >= 8192) ? CTFPOOLSZ : 0));
		if ((poolsz > 0) &&
		    (osl_ctfpool_init(osh, poolsz, RXBUFSZ+BCMEXTRAHDROOM) < 0)) {
			ET_ERROR(("et%d: chipattach: ctfpool alloc/init failed\n", unit));
			goto fail;
		}
	}
#endif /* CTFPOOL */

	bcopy(&et->etc->cur_etheraddr, dev->dev_addr, ETHER_ADDR_LEN);

	/* init 1 second watchdog timer */
	init_timer(&et->timer);
	et->timer.data = (ulong)dev;
	et->timer.function = et_watchdog;

#ifdef	NAPI2_POLL
	netif_napi_add(dev, & et->napi_poll, et_poll, 64);
	napi_enable(&et->napi_poll);
#endif	/* NAPI2_POLL */

#if !defined(NAPI_POLL) && !defined(NAPI2_POLL)
	/* setup the bottom half handler */
	tasklet_init(&et->tasklet, et_dpc, (ulong)et);
#endif /*! NAPIx_POLL */

#ifdef ET_ALL_PASSIVE
	if (ET_ALL_PASSIVE_ENAB(et)) {
		MY_INIT_WORK(&et->dpc_task.work, (work_func_t)et_dpc_work);
		et->dpc_task.context = et;
		MY_INIT_WORK(&et->txq_task.work, (work_func_t)et_txq_work);
		et->txq_task.context = et;
	}
	et->all_dispatch_mode = (passivemode == 0) ? TRUE : FALSE;
#endif  /* ET_ALL_PASSIVE */

	/* register our interrupt handler */
	if (request_irq(pdev->irq, et_isr, IRQF_SHARED, dev->name, et)) {
		ET_ERROR(("et%d: request_irq() failed\n", unit));
		goto fail;
	}
	dev->irq = pdev->irq;

	/* add us to the global linked list */
	et->next = et_list;
	et_list = et;

#ifndef HAVE_NET_DEVICE_OPS
	/* lastly, enable our entry points */
	dev->open = et_open;
	dev->stop = et_close;
	dev->hard_start_xmit = et_start;
	dev->get_stats = et_get_stats;
	dev->set_mac_address = et_set_mac_address;
	dev->set_multicast_list = et_set_multicast_list;
	dev->do_ioctl = et_ioctl;
#ifdef NAPI_POLL
	dev->poll = et_poll;
	dev->weight = (ET_GMAC(et->etc) ? 64 : 32);
#endif /* NAPI_POLL */
#else /* ! HAVE_NET_DEVICE_OPS */
	/* Linux 2.6.36 and up. - LR */
	dev->netdev_ops = &et_netdev_ops;
#ifdef NAPI_POLL
	dev->poll = et_poll;
	dev->weight = (ET_GMAC(et->etc) ? 64 : 32);
#endif /* NAPI_POLL */

#endif /* HAVE_NET_DEVICE_OPS */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	dev->ethtool_ops = &et_ethtool_ops;
#endif

	if (register_netdev(dev)) {
		ET_ERROR(("et%d: register_netdev() failed\n", unit));
		goto fail;
	}

	/* print hello string */
	(*et->etc->chops->longname)(et->etc->ch, name, sizeof(name));
	printf("%s: %s %s\n", dev->name, name, EPI_VERSION_STR);

#ifdef HNDCTF
	if (ctf_enable(et->cih, dev, TRUE, &et->brc_hot) != BCME_OK) {
		ET_ERROR(("et%d: ctf_enable() failed\n", unit));
		goto fail;
	}
#endif

	return (0);

fail:
	et_free(et);
	return (-ENODEV);
}

static int
et_suspend(struct pci_dev *pdev, DRV_SUSPEND_STATE_TYPE state)
{
	et_info_t *et;

	if ((et = (et_info_t *) pci_get_drvdata(pdev))) {
		netif_device_detach(et->dev);
		ET_LOCK(et);
		et_down(et, 1);
		ET_UNLOCK(et);
	}

	return 0;
}

static int
et_resume(struct pci_dev *pdev)
{
	et_info_t *et;

	if ((et = (et_info_t *) pci_get_drvdata(pdev))) {
		ET_LOCK(et);
		et_up(et);
		ET_UNLOCK(et);
		netif_device_attach(et->dev);
	}

	return 0;
}

/* Compatibility routines */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 6)
static void
_et_suspend(struct pci_dev *pdev)
{
	et_suspend(pdev, 0);
}

static void
_et_resume(struct pci_dev *pdev)
{
	et_resume(pdev);
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 6) */

static void __devexit
et_remove(struct pci_dev *pdev)
{
	et_info_t *et;

	if (!etc_chipmatch(pdev->vendor, pdev->device))
		return;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 12)
	et_suspend(pdev, 0);
#else
	et_suspend(pdev, PMSG_SUSPEND);
#endif

	if ((et = (et_info_t *) pci_get_drvdata(pdev))) {
		et_free(et);
		pci_set_drvdata(pdev, NULL);
	}
}

static struct pci_driver et_pci_driver = {
	name:		"et",
	probe:		et_probe,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 6)
	suspend:	_et_suspend,
	resume:		_et_resume,
#else
	suspend:	et_suspend,
	resume:		et_resume,
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 4, 6) */
	remove:		__devexit_p(et_remove),
	id_table:	et_id_table,
	};

static int __init
et_module_init(void)
{
#if defined(BCMDBG)
	if (msglevel != 0xdeadbeef)
		et_msg_level = msglevel;
	else {
		char *var = getvar(NULL, "et_msglevel");
		if (var)
			et_msg_level = bcm_strtoul(var, NULL, 0);
	}

	printf("%s: msglevel set to 0x%x\n", __FUNCTION__, et_msg_level);
#endif /* defined(BCMDBG) */

#ifdef ET_ALL_PASSIVE
	{
		char *var = getvar(NULL, "et_dispatch_mode");
		if (var)
			passivemode = bcm_strtoul(var, NULL, 0);
		printf("%s: passivemode set to 0x%x\n", __FUNCTION__, passivemode);
	}
#endif /* ET_ALL_PASSIVE */
#ifdef ET_LIMIT_TXQ
	{
		char *var = getvar(NULL, "et_txq_thresh");
		if (var)
			et_txq_thresh = bcm_strtoul(var, NULL, 0);
		printf("%s: et_txq_thresh set to 0x%x\n", __FUNCTION__, et_txq_thresh);
	}
#endif /* ET_LIMIT_TXQ */

	return pci_module_init(&et_pci_driver);
}

static void __exit
et_module_exit(void)
{
	pci_unregister_driver(&et_pci_driver);
}

module_init(et_module_init);
module_exit(et_module_exit);

static void
et_free(et_info_t *et)
{
	et_info_t **prev;
	osl_t *osh;

	if (et == NULL)
		return;

	ET_TRACE(("et: et_free\n"));

	if (et->dev && et->dev->irq)
		free_irq(et->dev->irq, et);

#ifdef	NAPI2_POLL
	napi_disable(&et->napi_poll);
	netif_napi_del(&et->napi_poll);
#endif	/* NAPI2_POLL */

#ifdef HNDCTF
	if (et->cih)
		ctf_dev_unregister(et->cih, et->dev);
#endif /* HNDCTF */

	if (et->dev) {
		unregister_netdev(et->dev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 0)
		free_netdev(et->dev);
#else
		MFREE(et->osh, et->dev, sizeof(struct net_device));
#endif
		et->dev = NULL;
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36) */
	}

#ifdef CTFPOOL
	/* free the buffers in fast pool */
	osl_ctfpool_cleanup(et->osh);
#endif /* CTFPOOL */

#ifdef HNDCTF
	/* free ctf resources */
	if (et->cih)
		ctf_detach(et->cih);
#endif /* HNDCTF */

	/* free common resources */
	if (et->etc) {
		etc_detach(et->etc);
		et->etc = NULL;
	}

	/*
	 * unregister_netdev() calls get_stats() which may read chip registers
	 * so we cannot unmap the chip registers until after calling unregister_netdev() .
	 */
	if (et->regsva) {
		iounmap((void *)et->regsva);
		et->regsva = NULL;
	}

	/* remove us from the global linked list */
	for (prev = &et_list; *prev; prev = &(*prev)->next)
		if (*prev == et) {
			*prev = et->next;
			break;
		}

	osh = et->osh;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
	free_netdev(et->dev);
	et->dev = NULL;
#else
	MFREE(et->osh, et, sizeof(et_info_t));
#endif

	if (MALLOCED(osh))
		printf("Memory leak of bytes %d\n", MALLOCED(osh));
	ASSERT(MALLOCED(osh) == 0);

	osl_detach(osh);
}

static int
et_open(struct net_device *dev)
{
	et_info_t *et;

	et = ET_INFO(dev);

	ET_TRACE(("et%d: et_open\n", et->etc->unit));

	et->etc->promisc = (dev->flags & IFF_PROMISC)? TRUE: FALSE;
	et->etc->allmulti = (dev->flags & IFF_ALLMULTI)? TRUE: et->etc->promisc;

	ET_LOCK(et);
	et_up(et);
	ET_UNLOCK(et);

	OLD_MOD_INC_USE_COUNT;

	return (0);
}

static int
et_close(struct net_device *dev)
{
	et_info_t *et;

	et = ET_INFO(dev);

	ET_TRACE(("et%d: et_close\n", et->etc->unit));

	et->etc->promisc = FALSE;
	et->etc->allmulti = FALSE;

	ET_LOCK(et);
	et_down(et, 1);
	ET_UNLOCK(et);

	OLD_MOD_DEC_USE_COUNT;

	return (0);
}

#ifdef ET_ALL_PASSIVE
static void BCMFASTPATH
et_txq_work(struct et_task *task)
{
	et_info_t *et = (et_info_t *)task->context;

	ET_LOCK(et);
	et_sendnext(et);
	ET_UNLOCK(et);

	return;
}
#endif /* ET_ALL_PASSIVE */

/*
 * Yeah, queueing the packets on a tx queue instead of throwing them
 * directly into the descriptor ring in the case of dma is kinda lame,
 * but this results in a unified transmit path for both dma and pio
 * and localizes/simplifies the netif_*_queue semantics, too.
 */
static int BCMFASTPATH
et_start(struct sk_buff *skb, struct net_device *dev)
{
	et_info_t *et;
	uint32 q = 0;
#ifdef ET_LIMIT_TXQ
	int qlen;
#endif /* ET_LIMIT_TXQ */

	et = ET_INFO(dev);

	if (ET_GMAC(et->etc) && (et->etc->qos))
		q = etc_up2tc(PKTPRIO(skb));

	ET_TRACE(("et%d: et_start: len %d\n", et->etc->unit, skb->len));
	ET_LOG("et%d: et_start: len %d", et->etc->unit, skb->len);

#ifdef ET_LIMIT_TXQ
	ET_TXQ_LOCK(et);
	qlen = skb_queue_len(&et->txq[q]);
	ET_TXQ_UNLOCK(et);
	if (et_txq_thresh && (qlen >= et_txq_thresh)) {
		PKTCFREE(et->osh, skb, TRUE);
		return 0;
	}
#endif /* ET_LIMIT_TXQ */

	/* put it on the tx queue and call sendnext */
	ET_TXQ_LOCK(et);
	__skb_queue_tail(&et->txq[q], skb);
	et->etc->txq_state |= (1 << q);
	ET_TXQ_UNLOCK(et);

	if (!ET_ALL_PASSIVE_ENAB(et)) {
		int locked = 0;
		if (!in_atomic()) {
			ET_LOCK(et);
			locked = 1;
		}
		et_sendnext(et);
		if (locked)
			ET_UNLOCK(et);
	}
#ifdef ET_ALL_PASSIVE
	else
		schedule_work(&et->txq_task.work);
#endif /* ET_ALL_PASSIVE */

	ET_LOG("et%d: et_start ret\n", et->etc->unit, 0);

	return (0);
}

static void BCMFASTPATH
et_sendnext(et_info_t *et)
{
	etc_info_t *etc;
	struct sk_buff *skb;
	void *p, *n;
	uint32 priq = TX_Q0;
	uint16 vlan_type;
#ifdef DMA
	uint32 txavail;
#endif

	etc = et->etc;

	ET_TRACE(("et%d: et_sendnext\n", etc->unit));
	ET_LOG("et%d: et_sendnext", etc->unit, 0);

	/* dequeue packets from highest priority queue and send */
	while (1) {
		ET_TXQ_LOCK(et);

		if (etc->txq_state == 0)
			break;

		priq = etc_priq(etc->txq_state);

		ET_TRACE(("et%d: txq_state %x priq %d txavail %d\n",
		          etc->unit, etc->txq_state, priq,
		          *(uint *)etc->txavail[priq]));

		if ((skb = skb_peek(&et->txq[priq])) == NULL) {
			etc->txq_state &= ~(1 << priq);
			ET_TXQ_UNLOCK(et);
			continue;
		}

#ifdef DMA
		/* current highest priority dma queue is full */
		txavail = *(uint *)(etc->txavail[priq]);
		if ((PKTISCHAINED(skb) && (txavail < PKTCCNT(skb))) || (txavail == 0))
#else /* DMA */
		if (etc->pioactive != NULL)
#endif /* DMA */
			break;

		skb = __skb_dequeue(&et->txq[priq]);

		ET_TXQ_UNLOCK(et);
		ET_PRHDR("tx", (struct ether_header *)skb->data, skb->len, etc->unit);
		ET_PRPKT("txpkt", skb->data, skb->len, etc->unit);

		/* convert the packet. */
		p = PKTFRMNATIVE(etc->osh, skb);
		ASSERT(p != NULL);

		ET_TRACE(("%s: sdu %p chained %d chain sz %d next %p\n",
		          __FUNCTION__, p, PKTISCHAINED(p), PKTCCNT(p), PKTCLINK(p)));

		vlan_type = ((struct ethervlan_header *)PKTDATA(et->osh, p))->vlan_type;

		FOREACH_CHAINED_PKT(p, n) {
			PKTCLRCHAINED(et->osh, p);
			/* replicate vlan header contents from curr frame */
			if ((n != NULL) && (vlan_type == HTON16(ETHER_TYPE_8021Q))) {
				uint8 *n_evh;
				n_evh = PKTPUSH(et->osh, n, VLAN_TAG_LEN);
				*(struct ethervlan_header *)n_evh =
				*(struct ethervlan_header *)PKTDATA(et->osh, p);
			}
			(*etc->chops->tx)(etc->ch, p);
			etc->txframe++;
			etc->txbyte += PKTLEN(et->osh, p);
		}
	}

	/* no flow control when qos is enabled */
	if (!et->etc->qos) {
		/* stop the queue whenever txq fills */
		if ((skb_queue_len(&et->txq[TX_Q0]) > DATAHIWAT) && !netif_queue_stopped(et->dev))
			netif_stop_queue(et->dev);
		else if (netif_queue_stopped(et->dev) &&
		         (skb_queue_len(&et->txq[TX_Q0]) < (DATAHIWAT/2)))
			netif_wake_queue(et->dev);
	} else {
		/* drop the frame if corresponding prec txq len exceeds hiwat
		 * when qos is enabled.
		 */
		if ((priq != TC_NONE) && (skb_queue_len(&et->txq[priq]) > DATAHIWAT)) {
			skb = __skb_dequeue(&et->txq[priq]);
			PKTCFREE(et->osh, skb, TRUE);
			ET_ERROR(("et%d: %s: txqlen %d\n", et->etc->unit,
			          __FUNCTION__, skb_queue_len(&et->txq[priq])));
		}
	}

	ET_TXQ_UNLOCK(et);
}

void
et_init(et_info_t *et, uint options)
{
	ET_TRACE(("et%d: et_init\n", et->etc->unit));
	ET_LOG("et%d: et_init", et->etc->unit, 0);

	etc_init(et->etc, options);
}


void
et_reset(et_info_t *et)
{
	ET_TRACE(("et%d: et_reset\n", et->etc->unit));
	etc_reset(et->etc);

	/* zap any pending dpc interrupt bits */
	et->events = 0;

	/* dpc will not be rescheduled */
	et->resched = 0;
}

void
et_up(et_info_t *et)
{
	etc_info_t *etc;

	etc = et->etc;

	if (etc->up)
		return;

	ET_TRACE(("et%d: et_up\n", etc->unit));

	etc_up(etc);

	/* schedule one second watchdog timer */
	et->timer.expires = jiffies + HZ;
	add_timer(&et->timer);
	et->set = TRUE;

	netif_start_queue(et->dev);
}

void
et_down(et_info_t *et, int reset)
{
	etc_info_t *etc;
	struct sk_buff *skb;
	int32 i;

	etc = et->etc;

	ET_TRACE(("et%d: et_down\n", etc->unit));

	netif_down(et->dev);
	netif_stop_queue(et->dev);

	/* stop watchdog timer */
	del_timer(&et->timer);
	et->set = FALSE;

	/* LR:
	 * Mark netif flag down,
	 * to prevent tasklet from rescheduling itself,
	 * and to prevent the ISR to scheduling any work items
	 */
	ET_FLAG_DOWN(etc);

#if !defined(NAPI_POLL) && !defined(NAPI2_POLL)
	/* kill dpc before cleaning out the queued buffers */
	ET_UNLOCK(et);
	tasklet_kill(&et->tasklet);
	ET_LOCK(et);
#endif /* NAPI_POLL */

#ifdef ET_ALL_PASSIVE
	/* Kill workqueue items too, or at least wait fo rthem to finisg */
#ifdef	__USE_GPL_SYMBOLS_
	/* LR:
	 * This is the right way to do this, but we can't unless we release the
	 * driver to GPL, which we're not allowed to.
	 */
	cancel_work_sync(&et->dpc_task.work);
	cancel_work_sync(&et->txq_task.work);
	cancel_work_sync(&et->wd_task.work);
#else
	flush_scheduled_work();
#endif
#endif /* ET_ALL_PASSIVE */

	/*
	 * LR: Now that all background activity ceased,
	 * it should be safe to proceed with shutdown
	 */

	/* flush the txq(s) */
	for (i = 0; i < NUMTXQ; i++)
		while ((skb = skb_dequeue(&et->txq[i])))
			PKTFREE(etc->osh, skb, TRUE);

	/* Shut down the hardware, reclaim Rx buffers */
	etc_down(etc, reset);

}

/*
 * These are interrupt on/off entry points. Disable interrupts
 * during interrupt state transition.
 */
void
et_intrson(et_info_t *et)
{
	unsigned long flags;
	INT_LOCK(flags);
	(*et->etc->chops->intrson)(et->etc->ch);
	INT_UNLOCK(flags);
}

static void
_et_watchdog(struct net_device *dev)
{
	et_info_t *et;

	et = ET_INFO(dev);

	ET_LOCK(et);

	etc_watchdog(et->etc);

	if (et->set) {
		/* reschedule one second watchdog timer */
		et->timer.expires = jiffies + HZ;
		add_timer(&et->timer);
	}

#ifdef CTFPOOL
	/* allocate and add a new skb to the pkt pool */
	if (CTF_ENAB(et->cih))
		osl_ctfpool_replenish(et->osh, CTFPOOL_REFILL_THRESH);
#endif /* CTFPOOL */
	ET_UNLOCK(et);
}

#ifdef ET_ALL_PASSIVE
static void
et_watchdog_task(et_task_t *task)
{
	_et_watchdog((struct net_device *)task->context);
}
#endif /* ET_ALL_PASSIVE */

static void
et_watchdog(ulong data)
{
	struct net_device *dev = (struct net_device *)data;
#ifdef ET_ALL_PASSIVE
	et_info_t *et = ET_INFO(dev);
#endif /* ET_ALL_PASSIVE */

	if (!ET_ALL_PASSIVE_ENAB(et))
		_et_watchdog(dev);
#ifdef ET_ALL_PASSIVE
	else {
		MY_INIT_WORK(&et->wd_task.work, (work_func_t)et_watchdog_task);
		et->wd_task.context = et;
	}
#endif /* ET_ALL_PASSIVE */
}


static int
et_get_settings(struct net_device *dev, struct ethtool_cmd *ecmd)
{
	et_info_t *et = ET_INFO(dev);

	ecmd->supported = (SUPPORTED_10baseT_Half | SUPPORTED_10baseT_Full |
	                   SUPPORTED_100baseT_Half | SUPPORTED_100baseT_Full |
	                   SUPPORTED_Autoneg);
	ecmd->advertising = ADVERTISED_TP;
	ecmd->advertising |= (et->etc->advertise & ADV_10HALF) ?
	        ADVERTISED_10baseT_Half : 0;
	ecmd->advertising |= (et->etc->advertise & ADV_10FULL) ?
	        ADVERTISED_10baseT_Full : 0;
	ecmd->advertising |= (et->etc->advertise & ADV_100HALF) ?
	        ADVERTISED_100baseT_Half : 0;
	ecmd->advertising |= (et->etc->advertise & ADV_100FULL) ?
	        ADVERTISED_100baseT_Full : 0;
	ecmd->advertising |= (et->etc->advertise2 & ADV_1000FULL) ?
	        ADVERTISED_1000baseT_Full : 0;
	ecmd->advertising |= (et->etc->advertise2 & ADV_1000HALF) ?
	        ADVERTISED_1000baseT_Half : 0;
	ecmd->advertising |= (et->etc->forcespeed == ET_AUTO) ?
	        ADVERTISED_Autoneg : 0;
	if (et->etc->linkstate) {
		ecmd->speed = (et->etc->speed == 1000) ? SPEED_1000 :
		               ((et->etc->speed == 100) ? SPEED_100 : SPEED_10);
		ecmd->duplex = (et->etc->duplex == 1) ? DUPLEX_FULL : DUPLEX_HALF;
	} else {
		ecmd->speed = 0;
		ecmd->duplex = 0;
	}
	ecmd->port = PORT_TP;
	ecmd->phy_address = 0;
	ecmd->transceiver = XCVR_INTERNAL;
	ecmd->autoneg = (et->etc->forcespeed == ET_AUTO) ? AUTONEG_ENABLE : AUTONEG_DISABLE;
	ecmd->maxtxpkt = 0;
	ecmd->maxrxpkt = 0;

	return 0;
}

static int
et_set_settings(struct net_device *dev, struct ethtool_cmd *ecmd)
{
	int speed;
	et_info_t *et = ET_INFO(dev);

	if (!capable(CAP_NET_ADMIN))
		return (-EPERM);

	else if (ecmd->speed == SPEED_10 && ecmd->duplex == DUPLEX_HALF)
		speed = ET_10HALF;
	else if (ecmd->speed == SPEED_10 && ecmd->duplex == DUPLEX_FULL)
		speed = ET_10FULL;
	else if (ecmd->speed == SPEED_100 && ecmd->duplex == DUPLEX_HALF)
		speed = ET_100HALF;
	else if (ecmd->speed == SPEED_100 && ecmd->duplex == DUPLEX_FULL)
		speed = ET_100FULL;
	else if (ecmd->speed == SPEED_1000 && ecmd->duplex == DUPLEX_FULL)
		speed = ET_1000FULL;
	else if (ecmd->autoneg == AUTONEG_ENABLE)
		speed = ET_AUTO;
	else
		return (-EINVAL);

	return etc_ioctl(et->etc, ETCSPEED, &speed);
}

static void
et_get_driver_info(struct net_device *dev, struct ethtool_drvinfo *info)
{
	et_info_t *et = ET_INFO(dev);

	bzero(info, sizeof(struct ethtool_drvinfo));
	info->cmd = ETHTOOL_GDRVINFO;
	sprintf(info->driver, "et%d", et->etc->unit);
	strncpy(info->version, EPI_VERSION_STR, sizeof(info->version));
	info->version[(sizeof(info->version))-1] = '\0';
}

#ifdef SIOCETHTOOL
static int
et_ethtool(et_info_t *et, struct ethtool_cmd *ecmd)
{
	int ret = 0;

	ET_LOCK(et);

	switch (ecmd->cmd) {
	case ETHTOOL_GSET:
		ret = et_get_settings(et->dev, ecmd);
		break;
	case ETHTOOL_SSET:
		ret = et_set_settings(et->dev, ecmd);
		break;
	case ETHTOOL_GDRVINFO:
		et_get_driver_info(et->dev, (struct ethtool_drvinfo *)ecmd);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	ET_UNLOCK(et);

	return (ret);
}
#endif /* SIOCETHTOOL */

static int
et_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	et_info_t *et;
	int error;
	char *buf;
	int size, ethtoolcmd;
	bool get = 0, set;
	et_var_t *var = NULL;
	void *buffer = NULL;

	et = ET_INFO(dev);

	ET_TRACE(("et%d: et_ioctl: cmd 0x%x\n", et->etc->unit, cmd));

	switch (cmd) {
#ifdef SIOCETHTOOL
	case SIOCETHTOOL:
		if (copy_from_user(&ethtoolcmd, ifr->ifr_data, sizeof(uint32)))
			return (-EFAULT);

		if (ethtoolcmd == ETHTOOL_GDRVINFO)
			size = sizeof(struct ethtool_drvinfo);
		else
			size = sizeof(struct ethtool_cmd);
		get = TRUE; set = TRUE;
		break;
#endif /* SIOCETHTOOL */
	case SIOCGETCDUMP:
		size = IOCBUFSZ;
		get = TRUE; set = FALSE;
		break;
	case SIOCGETCPHYRD:
	case SIOCGETCPHYRD2:
	case SIOCGETCROBORD:
		size = sizeof(int) * 2;
		get = TRUE; set = TRUE;
		break;
	case SIOCSETCPHYWR:
	case SIOCSETCPHYWR2:
	case SIOCSETCROBOWR:
		size = sizeof(int) * 2;
		get = FALSE; set = TRUE;
		break;
	case SIOCSETGETVAR:
		size = sizeof(et_var_t);
		set = TRUE;
		break;
	default:
		size = sizeof(int);
		get = FALSE; set = TRUE;
		break;
	}

	if ((buf = MALLOC(et->osh, size)) == NULL) {
		ET_ERROR(("et: et_ioctl: out of memory, malloced %d bytes\n", MALLOCED(et->osh)));
		return (-ENOMEM);
	}

	if (set && copy_from_user(buf, ifr->ifr_data, size)) {
		MFREE(et->osh, buf, size);
		return (-EFAULT);
	}

	if (cmd == SIOCSETGETVAR) {
		var = (et_var_t *)buf;
		if (var->buf) {
			if (!var->set)
				get = TRUE;

			if (!(buffer = (void *) MALLOC(et->osh, var->len))) {
				ET_ERROR(("et: et_ioctl: out of memory, malloced %d bytes\n",
					MALLOCED(et->osh)));
				MFREE(et->osh, buf, size);
				return (-ENOMEM);
			}

			if (copy_from_user(buffer, var->buf, var->len)) {
				MFREE(et->osh, buffer, var->len);
				MFREE(et->osh, buf, size);
				return (-EFAULT);
			}
		}
	}

	switch (cmd) {
#ifdef SIOCETHTOOL
	case SIOCETHTOOL:
		error = et_ethtool(et, (struct ethtool_cmd *)buf);
		break;
#endif /* SIOCETHTOOL */
	case SIOCSETGETVAR:
		ET_LOCK(et);
		error = etc_iovar(et->etc, var->cmd, var->set, buffer);
		ET_UNLOCK(et);
		if (!error && get)
			error = copy_to_user(var->buf, buffer, var->len);

		if (buffer)
			MFREE(et->osh, buffer, var->len);
		break;
	default:
		ET_LOCK(et);
		error = etc_ioctl(et->etc, cmd - SIOCSETCUP, buf) ? -EINVAL : 0;
		ET_UNLOCK(et);
		break;
	}

	if (!error && get)
		error = copy_to_user(ifr->ifr_data, buf, size);

	MFREE(et->osh, buf, size);

	return (error);
}

static struct net_device_stats *
et_get_stats(struct net_device *dev)
{
	et_info_t *et;
	etc_info_t *etc;
	struct net_device_stats *stats;
	int locked = 0;

	et = ET_INFO(dev);

	ET_TRACE(("et%d: et_get_stats\n", et->etc->unit));

	if (!in_atomic()) {
		locked = 1;
		ET_LOCK(et);
	}

	etc = et->etc;
	stats = &et->stats;
	bzero(stats, sizeof(struct net_device_stats));

	/* refresh stats */
	if (et->etc->up)
		(*etc->chops->statsupd)(etc->ch);

	/* SWAG */
	stats->rx_packets = etc->rxframe;
	stats->tx_packets = etc->txframe;
	stats->rx_bytes = etc->rxbyte;
	stats->tx_bytes = etc->txbyte;
	stats->rx_errors = etc->rxerror;
	stats->tx_errors = etc->txerror;

	if (ET_GMAC(etc)) {
		gmacmib_t *mib;

		mib = etc->mib;
		stats->collisions = mib->tx_total_cols;
		stats->rx_length_errors = (mib->rx_oversize_pkts + mib->rx_undersize);
		stats->rx_crc_errors = mib->rx_crc_errs;
		stats->rx_frame_errors = mib->rx_align_errs;
		stats->rx_missed_errors = mib->rx_missed_pkts;
	} else {
		bcmenetmib_t *mib;

		mib = etc->mib;
		stats->collisions = mib->tx_total_cols;
		stats->rx_length_errors = (mib->rx_oversize_pkts + mib->rx_undersize);
		stats->rx_crc_errors = mib->rx_crc_errs;
		stats->rx_frame_errors = mib->rx_align_errs;
		stats->rx_missed_errors = mib->rx_missed_pkts;

	}

	stats->rx_fifo_errors = etc->rxoflo;
	stats->rx_over_errors = etc->rxoflo;
	stats->tx_fifo_errors = etc->txuflo;

	if (locked)
		ET_UNLOCK(et);

	return (stats);
}

static int
et_set_mac_address(struct net_device *dev, void *addr)
{
	et_info_t *et;
	struct sockaddr *sa = (struct sockaddr *) addr;

	et = ET_INFO(dev);
	ET_TRACE(("et%d: et_set_mac_address\n", et->etc->unit));

	if (et->etc->up)
		return -EBUSY;

	bcopy(sa->sa_data, dev->dev_addr, ETHER_ADDR_LEN);
	bcopy(dev->dev_addr, &et->etc->cur_etheraddr, ETHER_ADDR_LEN);

	return 0;
}

static void
et_set_multicast_list(struct net_device *dev)
{
	et_info_t *et;
	etc_info_t *etc;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
	struct dev_mc_list *mclist;
#else
	struct netdev_hw_addr *ha;
#endif
	int i;
	int locked = 0;

	et = ET_INFO(dev);
	etc = et->etc;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
	mclist = NULL ;		/* fend off warnings */
#else
	ha = NULL;
#endif

	ET_TRACE(("et%d: et_set_multicast_list\n", etc->unit));

	if (!in_atomic()) {
		locked = 1;
		ET_LOCK(et);
	}

	if (etc->up) {
		etc->promisc = (dev->flags & IFF_PROMISC)? TRUE: FALSE;
		etc->allmulti = (dev->flags & IFF_ALLMULTI)? TRUE: etc->promisc;

		/* copy the list of multicasts into our private table */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
		for (i = 0, mclist = dev->mc_list; mclist && (i < dev->mc_count);
			i++, mclist = mclist->next) {
			if (i >= MAXMULTILIST) {
				etc->allmulti = TRUE;
				i = 0;
				break;
			}
			etc->multicast[i] = *((struct ether_addr *)mclist->dmi_addr);
		}
#else	/* >= 2.6.36 */
		i = 0;
		netdev_for_each_mc_addr(ha, dev) {
			i ++;
			if (i >= MAXMULTILIST) {
				etc->allmulti = TRUE; i = 0;
				i = 0;
				break;
			}
			etc->multicast[i] = *((struct ether_addr *)ha->addr);
		} /* for each ha */
#endif /* LINUX_VERSION_CODE */
		etc->nmulticast = i;

		/* LR: partial re-init, DMA is already initialized */
		et_init(et, ET_INIT_INTRON);
	}

	if (locked)
		ET_UNLOCK(et);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
static irqreturn_t BCMFASTPATH
et_isr(int irq, void *dev_id)
#else
static irqreturn_t BCMFASTPATH
et_isr(int irq, void *dev_id, struct pt_regs *ptregs)
#endif
{
	et_info_t *et;
	struct chops *chops;
	void *ch;
	uint events = 0;

	et = (et_info_t *)dev_id;
	chops = et->etc->chops;
	ch = et->etc->ch;

	/* guard against shared interrupts */
	if (!et->etc->up)
		goto done;

	/* get interrupt condition bits */
	events = (*chops->getintrevents)(ch, TRUE);


	/* not for us */
	if (!(events & INTR_NEW))
		goto done;

	ET_TRACE(("et%d: et_isr: events 0x%x\n", et->etc->unit, events));
	ET_LOG("et%d: et_isr: events 0x%x", et->etc->unit, events);

	/* disable interrupts */
	(*chops->intrsoff)(ch);

	/* save intstatus bits */
	ASSERT(et->events == 0);
	et->events = events;

	ASSERT(et->resched == FALSE);

#ifdef NAPI2_POLL

	napi_schedule(&et->napi_poll);

#elif defined(NAPI_POLL)
	/* allow the device to be added to the cpu polling list if we are up */
	if (netif_rx_schedule_prep(et->dev)) {
		/* tell the network core that we have packets to send up */
		__netif_rx_schedule(et->dev);
	} else {
		ET_ERROR(("et%d: et_isr: intr while in poll!\n",
		          et->etc->unit));
		(*chops->intrson)(ch);
	}
#else /* ! NAPI_POLL && ! NAPI2_POLL */
	/* schedule dpc */
#ifdef ET_ALL_PASSIVE
	if (ET_ALL_PASSIVE_ENAB(et)) {
		schedule_work(&et->dpc_task.work);
	} else
#endif /* ET_ALL_PASSIVE */
	tasklet_schedule(&et->tasklet);
#endif /* NAPI_POLL */

done:
	ET_LOG("et%d: et_isr ret", et->etc->unit, 0);

	return IRQ_RETVAL(events & INTR_NEW);
}

#ifdef PKTC
static void BCMFASTPATH
et_sendup_chain(et_info_t *et, void *h)
{
	struct sk_buff *skb;
	uint sz = PKTCCNT(h);

	ASSERT(h != NULL);

	skb = PKTTONATIVE(et->etc->osh, h);
	skb->dev = et->dev;

	ASSERT((sz > 0) && (sz <= PKTCBND));
	ET_TRACE(("et%d: %s: sending up packet chain of sz %d\n",
	          et->etc->unit, __FUNCTION__, sz));
	et->etc->chained += sz;
#ifdef BCMDBG
	et->etc->chainsz[sz - 1] += sz;
#endif
	et->etc->currchainsz = sz;
	et->etc->maxchainsz = MAX(et->etc->maxchainsz, sz);

	/* send up the packet chain */
	ctf_forward(et->cih, h, et->dev);
}
#endif /* PKTC */

static inline int
et_rxevent(osl_t *osh, et_info_t *et, struct chops *chops, void *ch, int quota)
{
	uint processed = 0;
	void *p, *h = NULL, *t = NULL;
	struct sk_buff *skb;
#ifdef PKTC
	pktc_data_t cd[PKTCMC] = {{0}};
	uint8 *evh, prio;
	int32 i = 0, cidx = 0;
	bool chaining = PKTC_ENAB(et);
#endif

	/* read the buffers first */
	while ((p = (*chops->rx)(ch))) {
#ifdef PKTC
		ASSERT(PKTCLINK(p) == NULL);
		evh = PKTDATA(et->osh, p) + HWRXOFF;
		prio = IP_TOS46(evh + ETHERVLAN_HDR_LEN) >> IPV4_TOS_PREC_SHIFT;
		if (cd[0].h_da == NULL) {
			cd[0].h_da = evh; cd[0].h_sa = evh + ETHER_ADDR_LEN;
			cd[0].h_prio = prio;
		}

		/* if current frame doesn't match cached src/dest/prio or has err flags
		 * set then stop chaining.
		 */
		if (chaining) {
			for (i = 0; i <= cidx; i++) {
				if (PKT_CHAINABLE(et, p, evh, prio, cd[i].h_sa,
				                  cd[i].h_da, cd[i].h_prio))
					break;
				else if ((i + 1 < PKTCMC) && (cd[i + 1].h_da == NULL)) {
					cidx++;
					cd[cidx].h_da = evh;
					cd[cidx].h_sa = evh + ETHER_ADDR_LEN;
					cd[cidx].h_prio = prio;
				}
			}
			chaining = (i < PKTCMC);
		}

		if (chaining) {
			PKTCENQTAIL(cd[i].chead, cd[i].ctail, p);
			/* strip off rxhdr */
			PKTPULL(et->osh, p, HWRXOFF);

			et->etc->rxframe++;
			et->etc->rxbyte += PKTLEN(et->osh, p);

			/* strip off crc32 */
			PKTSETLEN(et->osh, p, PKTLEN(et->osh, p) - ETHER_CRC_LEN);

			/* update header for non-first frames */
			if (cd[i].chead != p)
				CTF_HOTBRC_L2HDR_PREP(et->osh, et->brc_hot, prio,
				                      PKTDATA(et->osh, p), p);

			PKTCINCRCNT(cd[i].chead);
			PKTSETCHAINED(et->osh, p);
			PKTCADDLEN(cd[i].chead, PKTLEN(et->osh, p));
		} else
			PKTCENQTAIL(h, t, p);
#else /* PKTC */
		PKTSETLINK(p, NULL);
		if (t == NULL)
			h = t = p;
		else {
			PKTSETLINK(t, p);
			t = p;
		}
#endif /* PKTC */

		/* we reached quota already */
		if (++processed >= quota) {
			/* reschedule et_dpc()/et_poll() */
			et->resched = TRUE;
			break;
		}
	}

	/* prefetch the headers */
	if (h != NULL)
		ETPREFHDRS(PKTDATA(osh, h), PREFSZ);

	/* post more rx bufs */
	(*chops->rxfill)(ch);

#ifdef PKTC
	/* send up the chain(s) at one fell swoop */
	ASSERT(cidx < PKTCMC);
	for (i = 0; i <= cidx; i++) {
		if (cd[i].chead != NULL)
			et_sendup_chain(et, cd[i].chead);
	}
#endif

	while ((p = h) != NULL) {
#ifdef PKTC
		h = PKTCLINK(h);
		PKTSETCLINK(p, NULL);
#else
		h = PKTLINK(h);
		PKTSETLINK(p, NULL);
#endif
		/* prefetch the headers */
		if (h != NULL)
			ETPREFHDRS(PKTDATA(osh, h), PREFSZ);
		skb = PKTTONATIVE(osh, p);
		et->etc->unchained++;
		et_sendup(et, skb);
	}

	return (processed);
}

#if defined(NAPI2_POLL)
static int BCMFASTPATH
et_poll(struct napi_struct *napi, int budget)
{
	int quota = budget;
	struct net_device *dev = napi->dev;
	et_info_t *et = ET_INFO(dev);

#elif defined(NAPI_POLL)
static int BCMFASTPATH
et_poll(struct net_device *dev, int *budget)
{
	int quota = min(RXBND, *budget);
	et_info_t *et = ET_INFO(dev);
#else /* NAPI_POLL */
static void BCMFASTPATH
et_dpc(ulong data)
{
	et_info_t *et = (et_info_t *)data;
	int quota = PKTC_ENAB(et) ? et->etc->pktcbnd : RXBND;
#endif /* NAPI_POLL */
	struct chops *chops;
	void *ch;
	osl_t *osh;
	uint nrx = 0;

	chops = et->etc->chops;
	ch = et->etc->ch;
	osh = et->etc->osh;

	ET_TRACE(("et%d: et_dpc: events 0x%x\n", et->etc->unit, et->events));
	ET_LOG("et%d: et_dpc: events 0x%x", et->etc->unit, et->events);

#if !defined(NAPI_POLL) && !defined(NAPI2_POLL)
	ET_LOCK(et);
#endif /* ! NAPIx_POLL */

	if (!et->etc->up)
		goto done;

	/* get interrupt condition bits again when dpc was rescheduled */
	if (et->resched) {
		et->events = (*chops->getintrevents)(ch, FALSE);
		et->resched = FALSE;
	}

	if (et->events & INTR_RX)
		nrx = et_rxevent(osh, et, chops, ch, quota);

	if (et->events & INTR_TX) {
		ET_TXQ_LOCK(et);
		(*chops->txreclaim)(ch, FALSE);
		ET_TXQ_UNLOCK(et);
	}

	(*chops->rxfill)(ch);

	/* handle error conditions, if reset required leave interrupts off! */
	if (et->events & INTR_ERROR) {
		if ((*chops->errors)(ch))
			et_init(et, ET_INIT_INTROFF);
		else
			if (nrx < quota)
				nrx += et_rxevent(osh, et, chops, ch, quota);
	}

	/* run the tx queue */
	if (et->etc->txq_state != 0) {
		if (!ET_ALL_PASSIVE_ENAB(et)) {
#ifndef CONFIG_SMP
			ET_LOCK(et);
			et_sendnext(et);
			ET_UNLOCK(et);
#else
			et_sendnext(et);
#endif /* CONFIG_SMP */
			}
#ifdef ET_ALL_PASSIVE
		else
			schedule_work(&et->txq_task.work);
#endif /* ET_ALL_PASSIVE */
		}

	/* clear this before re-enabling interrupts */
	et->events = 0;

	/* something may bring the driver down */
	if (!et->etc->up) {
		et->resched = FALSE;
		goto done;
	}

#if !defined(NAPI_POLL) && !defined(NAPI2_POLL)
#ifdef ET_ALL_PASSIVE
	if (et->resched) {
		if (!ET_ALL_PASSIVE_ENAB(et))
			tasklet_schedule(&et->tasklet);
		else
			schedule_work(&et->dpc_task.work);
	}
	else
		(*chops->intrson)(ch);
#else /* ET_ALL_PASSIVE */
	/* there may be frames left, reschedule et_dpc() */
	if (et->resched)
		tasklet_schedule(&et->tasklet);
	/* re-enable interrupts */
	else
		(*chops->intrson)(ch);
#endif /* ET_ALL_PASSIVE */
#endif /* ! NAPIx_POLL */

done:
	ET_LOG("et%d: et_dpc ret", et->etc->unit, 0);

#if defined(NAPI_POLL) || defined(NAPI2_POLL)
#ifdef	NAPI_POLL
	/* update number of frames processed */
	*budget -= nrx;
	dev->quota -= nrx;

	ET_TRACE(("et%d: et_poll: quota %d budget %d\n",
	          et->etc->unit, dev->quota, *budget));
#else
	ET_TRACE(("et%d: et_poll: budget %d\n",
	          et->etc->unit, budget));
#endif

	/* we got packets but no quota */
	if (et->resched)
		/* indicate that we are not done, don't enable
		 * interrupts yet. linux network core will call
		 * us again.
		 */
		return (1);

#ifdef	NAPI2_POLL
	napi_complete(napi);
#else	/* NAPI_POLL */
	netif_rx_complete(dev);
#endif

	/* enable interrupts now */
	(*chops->intrson)(ch);

	/* indicate that we are done */
	return (0);
#else /* NAPI_POLL */
	ET_UNLOCK(et);
	return;
#endif /* NAPI_POLL */
}

#ifdef ET_ALL_PASSIVE
static void BCMFASTPATH
et_dpc_work(struct et_task *task)
{
#if !defined(NAPI_POLL) && !defined(NAPI2_POLL)
	et_info_t *et = (et_info_t *)task->context;
	et_dpc((unsigned long)et);
#else
	BUG_ON(1);
#endif
	return;
}
#endif /* ET_ALL_PASSIVE */

static void
et_error(et_info_t *et, struct sk_buff *skb, void *rxh)
{
	uchar eabuf[32];
	struct ether_header *eh;

	eh = (struct ether_header *)skb->data;
	bcm_ether_ntoa((struct ether_addr *)eh->ether_shost, eabuf);

	if (RXH_OVERSIZE(et->etc, rxh)) {
		ET_ERROR(("et%d: rx: over size packet from %s\n", et->etc->unit, eabuf));
	}
	if (RXH_CRC(et->etc, rxh)) {
		ET_ERROR(("et%d: rx: crc error from %s\n", et->etc->unit, eabuf));
	}
	if (RXH_OVF(et->etc, rxh)) {
		ET_ERROR(("et%d: rx: fifo overflow\n", et->etc->unit));
	}
	if (RXH_NO(et->etc, rxh)) {
		ET_ERROR(("et%d: rx: crc error (odd nibbles) from %s\n",
		          et->etc->unit, eabuf));
	}
	if (RXH_RXER(et->etc, rxh)) {
		ET_ERROR(("et%d: rx: symbol error from %s\n", et->etc->unit, eabuf));
	}
}

#ifdef CONFIG_IP_NF_DNSMQ
typedef int (*dnsmqHitHook)(struct sk_buff *skb);
extern dnsmqHitHook dnsmq_hit_hook;
#endif

static inline int32
et_ctf_forward(et_info_t *et, struct sk_buff *skb)
{
#ifdef CONFIG_IP_NF_DNSMQ
        if(dnsmq_hit_hook&&dnsmq_hit_hook(skb))
                return (BCME_ERROR);
#endif
#ifdef HNDCTF
	/* use slow path if ctf is disabled */
	if (!CTF_ENAB(et->cih))
		return (BCME_ERROR);

	/* try cut thru first */
	if (ctf_forward(et->cih, skb, skb->dev) != BCME_ERROR)
		return (BCME_OK);

	/* clear skipct flag before sending up */
	PKTCLRSKIPCT(et->osh, skb);
#endif /* HNDCTF */

#ifdef CTFPOOL
	/* allocate and add a new skb to the pkt pool */
	if (PKTISFAST(et->osh, skb))
		osl_ctfpool_add(et->osh);

	/* clear fast buf flag before sending up */
	PKTCLRFAST(et->osh, skb);

	/* re-init the hijacked field */
	CTFPOOLPTR(et->osh, skb) = NULL;
#endif /* CTFPOOL */

	/* map the unmapped buffer memory before sending up */
	PKTCTFMAP(et->osh, skb);

	return (BCME_ERROR);
}

void BCMFASTPATH
et_sendup(et_info_t *et, struct sk_buff *skb)
{
	etc_info_t *etc;
	void *rxh;
	uint16 flags;

	etc = et->etc;

	/* packet buffer starts with rxhdr */
	rxh = skb->data;

	/* strip off rxhdr */
	__skb_pull(skb, HWRXOFF);

	ET_TRACE(("et%d: et_sendup: %d bytes\n", et->etc->unit, skb->len));
	ET_LOG("et%d: et_sendup: len %d", et->etc->unit, skb->len);

	etc->rxframe++;
	etc->rxbyte += skb->len;

	/* eh should now be aligned 2-mod-4 */
	ASSERT(((ulong)skb->data & 3) == 2);

	/* strip off crc32 */
	__skb_trim(skb, skb->len - ETHER_CRC_LEN);

	ET_PRHDR("rx", (struct ether_header *)skb->data, skb->len, etc->unit);
	ET_PRPKT("rxpkt", skb->data, skb->len, etc->unit);

	/* get the error flags */
	flags = RXH_FLAGS(etc, rxh);

	/* check for reported frame errors */
	if (flags)
		goto err;

	skb->dev = et->dev;

#ifdef HNDCTF
	/* try cut thru' before sending up */
	if (et_ctf_forward(et, skb) != BCME_ERROR)
		return;
#endif /* HNDCTF */

	ASSERT(!PKTISCHAINED(skb));
	ASSERT(PKTCGETATTR(skb) == 0);

	/* extract priority from payload and store it out-of-band
	 * in skb->priority
	 */
	if (et->etc->qos)
		pktsetprio(skb, TRUE);

	skb->protocol = eth_type_trans(skb, et->dev);

	/* send it up */
#if	defined(NAPI_POLL) || defined(NAPI2_POLL)
	netif_receive_skb(skb);
#else /* NAPI_POLL */
	netif_rx(skb);
#endif /* NAPI_POLL */

	ET_LOG("et%d: et_sendup ret", et->etc->unit, 0);

	return;

err:
	et_error(et, skb, rxh);
	PKTFRMNATIVE(etc->osh, skb);
	PKTFREE(etc->osh, skb, FALSE);

	return;
}

#ifdef HNDCTF
void
et_dump_ctf(et_info_t *et, struct bcmstrbuf *b)
{
	ctf_dump(et->cih, b);
}
#endif

void
et_dump(et_info_t *et, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "et%d: %s %s version %s\n", et->etc->unit,
		__DATE__, __TIME__, EPI_VERSION_STR);

#ifdef HNDCTF
#if defined(BCMDBG)
	ctf_dump(et->cih, b);
#endif 
#endif /* HNDCTF */

#ifdef BCMDBG
	et_dumpet(et, b);
	etc_dump(et->etc, b);
#endif /* BCMDBG */
}

#ifdef BCMDBG
static void
et_dumpet(et_info_t *et, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "et %p dev %p name %s tbusy %d txq[0].qlen %d malloced %d\n",
		et, et->dev, et->dev->name, (uint)netif_queue_stopped(et->dev), et->txq[0].qlen,
		MALLOCED(et->osh));
}
#endif	/* BCMDBG */

void
et_link_up(et_info_t *et)
{
	ET_ERROR(("et%d: link up (%d%s)\n",
		et->etc->unit, et->etc->speed, (et->etc->duplex? "FD" : "HD")));
}

void
et_link_down(et_info_t *et)
{
	ET_ERROR(("et%d: link down\n", et->etc->unit));
}

/*
 * 47XX-specific shared mdc/mdio contortion:
 * Find the et associated with the same chip as <et>
 * and coreunit matching <coreunit>.
 */
void *
et_phyfind(et_info_t *et, uint coreunit)
{
	et_info_t *tmp;
	uint bus, slot;

	bus = et->pdev->bus->number;
	slot = PCI_SLOT(et->pdev->devfn);

	/* walk the list et's */
	for (tmp = et_list; tmp; tmp = tmp->next) {
		if (et->etc == NULL)
			continue;
		if (tmp->pdev == NULL)
			continue;
		if (tmp->pdev->bus->number != bus)
			continue;
		if (tmp->etc->nicmode)
			if (PCI_SLOT(tmp->pdev->devfn) != slot)
				continue;
		if (tmp->etc->coreunit != coreunit)
			continue;
		break;
	}
	return (tmp);
}

/* shared phy read entry point */
uint16
et_phyrd(et_info_t *et, uint phyaddr, uint reg)
{
	uint16 val;

	ET_LOCK(et);
	val = et->etc->chops->phyrd(et->etc->ch, phyaddr, reg);
	ET_UNLOCK(et);

	return (val);
}

/* shared phy write entry point */
void
et_phywr(et_info_t *et, uint phyaddr, uint reg, uint16 val)
{
	ET_LOCK(et);
	et->etc->chops->phywr(et->etc->ch, phyaddr, reg, val);
	ET_UNLOCK(et);
}
